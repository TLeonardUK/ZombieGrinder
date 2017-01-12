// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/MarkupFontRenderer.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Renderer/RenderBatch.h"

#include "Engine/Renderer/Textures/Texture.h"
#include "Engine/Renderer/Textures/TextureFactory.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Input/Input.h"
#include "Engine/Input/InputBindings.h"
#include "Engine/Input/OutputBindings.h"

#include "Engine/Online/OnlineUser.h"
#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineClient.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/Renderer/PrimitiveRenderer.h"

#include "Engine/Engine/EngineOptions.h"

#include "Engine/Display/GfxDisplay.h"

#include "Engine/Renderer/Text/TagReplacer.h"

#include "Engine/Scene/Camera.h"

#include "Generic/Helper/StringHelper.h"

#include "Generic/Math/Math.h"

void MarkupParser::Start(const char* text)
{
	m_offset = 0;
	m_text = text;
	m_length = strlen(text);
}

void MarkupParser::Parse_Tag(MarkupToken& token)
{
	int offset = token.Value.find('=');
	if (offset != std::string::npos)
	{
		std::string params = token.Value.substr(offset + 1);
		token.Value = token.Value.substr(0, offset);

		offset = 0;
		std::string param = "";
		while (offset != params.length())
		{
			char next = params[offset];

			if (next == ',')
			{
				token.Parameters.push_back(Variant(param));
				param = "";
			}
			else
			{
				param += next;
			}

			offset++;
		}

		if (param != "")
		{
			token.Parameters.push_back(Variant(param));
			param = "";
		}
	}
	else
	{
		token.Parameters.clear();
	}
}

void MarkupParser::Next(MarkupToken& token)
{
	token.Value = "";

	while (true)
	{
		// Are we at the end yet?
		if (m_offset == m_length)
		{
			if (token.Value != "")
			{
				token.Type = MarkupTokenType::Text;
				return;
			}
			else
			{
				token.Type = MarkupTokenType::End;
				return;
			}
		}

		// Eat up next character.
		char next = m_text[m_offset];
		if (next == '[')
		{
			// Escaped tag?
			if (m_offset < m_length - 1 &&
				m_text[m_offset + 1] == '[')
			{
				token.Value += '[';
				m_offset += 2;
			}			
			else if (token.Value != "")
			{
				token.Type = MarkupTokenType::Text;
				return;
			}
			else
			{
				m_offset++;

				// Try to parse till end of tag.
				while (true)
				{
					if (m_offset == m_length)
					{
						if (token.Value != "")
						{
							token.Type = MarkupTokenType::Text;
							return;
						}
						else
						{
							token.Type = MarkupTokenType::End;
							return;
						}
					}

					next = m_text[m_offset];
					if (next == ']')
					{
						m_offset++;
						token.Type = MarkupTokenType::Tag;
						Parse_Tag(token);
						return;
					}
					else
					{
						token.Value += next;
						m_offset++;
					}
				}
			}
		}
		else
		{
			token.Value += next;
			m_offset++;
		}
	}
}

MarkupFontRenderer::MarkupFontRenderer()
	: m_font(NULL)
	, m_persist_state(false)
{
	Clear_State();
}

MarkupFontRenderer::MarkupFontRenderer(FontHandle* font, bool buffered, bool shadow, bool persist_state)
	: m_font(font)
	, m_font_renderer(font, buffered, shadow)
	, m_persist_state(persist_state)
{
	Clear_State();
}

MarkupFontRenderer::~MarkupFontRenderer()
{
}

void MarkupFontRenderer::Clear_State()
{
	m_color_stack.clear();
	m_fade_stack.clear();
	m_bold_depth = 0;
}

void MarkupFontRenderer::Draw(FontRenderState& state, const char* text, Vector2 location, float font_height, Color color, Vector2 extra_spacing, Vector2 output_scale, RendererOption::Type blend)
{
	bool done = false;
	
	//float font_scale = 0.5f;//(font_height * output_scale.Y)  / m_font->Get()->Get_Base_Height();
	float line_height = m_font_renderer.Calculate_String_Size(" ", font_height, output_scale).Y;
	float global_scale = color.A / 255.0f;

	// Effect stacks.
	if (m_persist_state == false)
	{
		Clear_State();
	}

	if (m_color_stack.size() <= 0)
	{
		m_color_stack.push_back(color);
	}

	if (m_fade_stack.size() <= 0)
	{
		m_fade_stack.push_back(1.0f);
	}

	// Paaaaaaaarse tags.
	MarkupParser parser;
	parser.Start(text);

	while (!done)
	{
		MarkupToken token;
		parser.Next(token);

		switch (token.Type)
		{
		case MarkupTokenType::Text:
			{
				Color& topcolor = m_color_stack.at(m_color_stack.size() - 1);
				double fade = m_fade_stack.at(m_fade_stack.size() - 1) * global_scale;
				Color output_color = Color((int)topcolor.R, (int)topcolor.G, (int)topcolor.B, (int)(topcolor.A * fade));
				m_font_renderer.Draw(state, token.Value.c_str(), font_height, output_color, extra_spacing, output_scale, blend);
			}
			break;
		case MarkupTokenType::Tag:
			{
				if (token.Value == "img")
				{
					if (token.Parameters.size() == 3 || token.Parameters.size() == 1 || token.Parameters.size() == 5)
					{
						AtlasFrame* frame = ResourceFactory::Get()->Get_Atlas_Frame(token.Parameters[0].Get_String().c_str());
						if (frame == NULL)
						{
							continue;
						}

						float scale_x = line_height / frame->Rect.Height;
						float scale_y = scale_x;
						float offset_x = 0;
						float offset_y = 0;

						if (token.Parameters.size() >= 3)
						{
							scale_x *= token.Parameters[1].Get_Float();
							scale_y *= token.Parameters[2].Get_Float();
						}

						if (token.Parameters.size() == 5)
						{
							offset_x = token.Parameters[3].Get_Float() * output_scale.X * scale_x;
							offset_y = token.Parameters[4].Get_Float() * output_scale.Y * scale_y;
						}
						
						Color& topcolor = m_color_stack.at(m_color_stack.size() - 1);
						double fade = m_fade_stack.at(m_fade_stack.size() - 1) * global_scale;
						Color output_color = Color((int)topcolor.R, (int)topcolor.G, (int)topcolor.B, (int)(topcolor.A * fade));

						AtlasRenderer renderer;
						renderer.Draw_Frame(
							frame, 
							Rect2D(state.Pen.X + (frame->Origin.X) + offset_x, state.Pen.Y + (frame->Origin.Y) + offset_y, frame->Rect.Width * scale_x, frame->Rect.Height * scale_y), 
							0.0f, 
							output_color, 
							false, 
							false, 
							blend
						);
				
//	PrimitiveRenderer pr;
//	pr.Draw_Solid_Quad(Rect2D(state.Pen.X + (frame->Origin.X) + offset_x, state.Pen.Y + (frame->Origin.Y) + offset_y, frame->Rect.Width * scale_x, frame->Rect.Height * scale_y), Color::Magenta);

						state.Pen = state.Pen + Vector2(frame->Rect.Width * scale_x, 0.0f);
					}
				}
				else if (token.Value == "avatar")
				{
					if (token.Parameters.size() == 3 || token.Parameters.size() == 1 || token.Parameters.size() == 5)
					{
						OnlineUser* user = OnlinePlatform::Get()->Get_User_By_ID(token.Parameters[0].Get_Int());
						if (user == NULL)
						{
							continue;
						}

						Texture* texture = user->Get_Avatar_Texture();
						if (texture == NULL)
						{
							continue;
						}
						
						float scale_x = line_height / texture->Get_Height();
						float scale_y = scale_x;
						float offset_x = 0;
						float offset_y = 0;

						if (token.Parameters.size() >= 3)
						{
							scale_x *= token.Parameters[1].Get_Float();
							scale_y *= token.Parameters[2].Get_Float();
						}

						if (token.Parameters.size() == 5)
						{
							offset_x = token.Parameters[3].Get_Float() * output_scale.X * scale_x;
							offset_y = token.Parameters[4].Get_Float() * output_scale.Y * scale_y;
						}
						
						Color& topcolor = m_color_stack.at(m_color_stack.size() - 1);
						double fade = m_fade_stack.at(m_fade_stack.size() - 1) * global_scale;
						Color output_color = Color((int)topcolor.R, (int)topcolor.G, (int)topcolor.B, (int)(topcolor.A * fade));

						Rect2D  uv = Rect2D(0.0f, 0.0f, 1.0f, 1.0f);
						Vector2 origin = Vector2(0, 0);
						Rect2D  box = Rect2D
						(
							state.Pen.X + offset_x, 
							state.Pen.Y + offset_y, 
							texture->Get_Width() * scale_x, 
							texture->Get_Height() * scale_y
						);

						AtlasRenderer renderer;
						renderer.Draw(
							texture, 
							uv, 
							origin, 
							box, 
							0.0f, 
							output_color, 
							false, 
							false, 
							blend
						);
				
						state.Pen = state.Pen + Vector2(texture->Get_Width() * scale_x, texture->Get_Height() * scale_y);
					}
				}
				else if (token.Value == "key")
				{
					if (token.Parameters.size() == 1 || token.Parameters.size() == 2)
					{	
						const char* output_name = token.Parameters[0].Get_String().c_str();
						OutputBindings::Type output;
						
						InputBindingType::Type type = InputBindingType::Any;
						if (token.Parameters.size() >= 2)
						{							
							const char* key_type = token.Parameters[1].Get_String().c_str();
							if (stricmp(key_type, "joystick") == 0)
							{
								type = InputBindingType::Joystick;
							}
							else if (stricmp(key_type, "keyboard") == 0)
							{
								type = InputBindingType::Keyboard;
							}
						}

						if (OutputBindings::Parse(output_name, output))
						{
							JoystickState* joystick_state = NULL;
							InputBindings::Type input = Input::Get()->Output_To_Active_Input(output, type, &joystick_state);
							std::string atlas_name = "";

							if (InputBindings::ToAtlasFrameName(input, atlas_name, Input::Get()->Get_Active_Joystick_Icon_Set()))
							{
								if (joystick_state && type != InputBindingType::Keyboard)
								{
									std::string result = joystick_state->Get_Custom_Action_Icon(output);
									if (result != "")
									{
									//	DBG_LOG("Atlas=%s", result.c_str());
										atlas_name = result;
									}
								}

								AtlasFrame* frame = ResourceFactory::Get()->Get_Atlas_Frame(atlas_name.c_str());
								if (frame != NULL)
								{
									Color& topcolor = m_color_stack.at(m_color_stack.size() - 1);
									double fade = m_fade_stack.at(m_fade_stack.size() - 1) * global_scale;
									Color output_color = Color((int)topcolor.R, (int)topcolor.G, (int)topcolor.B, (int)(topcolor.A * fade));
									
									float key_scale = 1.5f;

									// HACK: Fix steam icons being to large. Should probably downsize the source files really -_-
									if (atlas_name.find("Steam") != std::string::npos)
									{
										key_scale = 1.0f;
									}

									float scaled_line_height = (line_height * key_scale);
									float line_height_diff = scaled_line_height - line_height;
									float scale_x = scaled_line_height / frame->Rect.Height;
									float scale_y = scale_x;

									// We apply a fake scale and vertically center key icons to make them more legible.

									Rect2D box = Rect2D
									(
										state.Pen.X, 
										state.Pen.Y - (line_height_diff * 0.5f),
										frame->Rect.Width * scale_x,
										(frame->Rect.Height * scale_y) 
									);
									
	//PrimitiveRenderer pr;
//	pr.Draw_Solid_Quad(box, Color::Magenta);
	
									AtlasRenderer renderer;
									renderer.Draw_Frame(frame, box, 0.0f, output_color, false, false, blend);
				
									state.Pen = state.Pen + Vector2(box.Width, 0.0f);
								}
							}
						}
					}
				}
				else if (token.Value == "b")
				{
					m_bold_depth++;
				}
				else if (token.Value == "/b")
				{
					m_bold_depth--;
				}
				else if (token.Value == "c")
				{
					if (token.Parameters.size() == 3)
					{
						Color& topcolor = m_color_stack.at(m_color_stack.size() - 1);
						m_color_stack.push_back(Color(token.Parameters[0].Get_Int(), token.Parameters[1].Get_Int(), token.Parameters[2].Get_Int(), topcolor.A));
					}
					else if (token.Parameters.size() == 4)
					{
						m_color_stack.push_back(Color(token.Parameters[0].Get_Int(), token.Parameters[1].Get_Int(), token.Parameters[2].Get_Int(), token.Parameters[3].Get_Int()));
					}
				}
				else if (token.Value == "/c")
				{
					if (token.Parameters.size() == 0)
					{
						if (m_color_stack.size() > 1)
						{
							m_color_stack.pop_back();
						}
					}
				}
				else if (token.Value == "fade")
				{
					if (token.Parameters.size() == 3)
					{
						double start_time = (float)atof(token.Parameters.at(0).Get_String().c_str()); 
						double delay      = (float)atof(token.Parameters.at(1).Get_String().c_str());
						double duration   = (float)atof(token.Parameters.at(2).Get_String().c_str());

						double elapsed    = Max(0.0, Platform::Get()->Get_Ticks() - (start_time + delay));
						double delta		 = Min(1.0, Max(0.0, elapsed / duration)); 

						m_fade_stack.push_back(1.0 - delta);
					}
				}
				else if (token.Value == "/fade")
				{
					if (token.Parameters.size() == 0)
					{
						if (m_fade_stack.size() > 1)
						{
							m_fade_stack.pop_back();
						}
					}
				}
			}
			break;
		case MarkupTokenType::End:
			{
				done = true;
			}
			break;
		}
	}
}

void MarkupFontRenderer::Draw_String(const char* text, Vector2 location, float font_height, Color color, Vector2 extra_spacing, Vector2 output_scale, RendererOption::Type blend)
{
	FontRenderState state = m_font_renderer.Begin_Draw(location);
	Draw(state, text, location, font_height, color, extra_spacing, output_scale, blend);
}

void MarkupFontRenderer::Draw_String(const char* text, Rect2D bounds, float font_height, Color color, TextAlignment::Type horizontal_align, TextAlignment::Type vertical_align, Vector2 output_scale, RendererOption::Type blend)
{ 
	// Round location, sub-pixel fonts look crap.
	bounds.X = floorf(bounds.X);
	bounds.Y = floorf(bounds.Y);
	bounds.Width = floorf(bounds.Width);
	bounds.Height = floorf(bounds.Height);

	// Measure string size and scale font to fit it in.
	Vector2 string_size = Calculate_String_Size(text, font_height, output_scale);

	float scale	= 1.0f;
	float line_spacing = font_height * 0.2f;

	// Scale text until it fits vertically.
	if (vertical_align == TextAlignment::ScaleToFit)
	{
		scale = bounds.Height / string_size.Y;
	}

	// If string height < bounds, scale down if possible.
	else
	{
		scale = Min(1.0f, bounds.Height / string_size.Y);
		if (scale == 0.0f)
		{
			scale = 1.0f;
		}
	}
	
//	PrimitiveRenderer pr;
//	pr.Draw_Solid_Quad(bounds, Color::Green);
	
	Vector2 final_scale = output_scale * scale;//output_scale; // scale is already scaled by output_scale, fuuu
	string_size = Vector2(string_size.X * scale, string_size.Y * scale);

	// Split into multiple lines.
	std::vector<std::string> lines;
	StringHelper::Split(text, '\n', lines);

	// Calculate Y-Offset based on alignment.
	float y_offset = 0.0f;
	float extra_spacing_y = 0.0f;
	switch (vertical_align)
	{
	case TextAlignment::Top:		
		{
			y_offset = 0;
			break;
		}
	case TextAlignment::Center:
		{
			y_offset = (bounds.Height / 2) - (string_size.Y / 2);
			break;
		}
	case TextAlignment::Bottom:		
		{
			y_offset = bounds.Height - string_size.Y;
			break;
		}
	case TextAlignment::Justified:
		{
			extra_spacing_y = (bounds.Height - string_size.Y) / lines.size();
			break;
		}
	case TextAlignment::ScaleToFit:
		{
			break;
		}
	default: DBG_ASSERT_STR(false, "Invalid vertical alignment.");
	}

	// Render each line individually with appropriate alignment.
	float pen_y = bounds.Y + y_offset;
	for (std::vector<std::string>::iterator iter = lines.begin(); iter != lines.end(); iter++)
	{
		std::string& line = *iter;

		// Horizontal alignment.
		switch (horizontal_align)
		{
		case TextAlignment::Left:	
			{
				Draw_String(line.c_str(), Vector2(bounds.X, pen_y), font_height, color, Vector2(0.0f, extra_spacing_y), final_scale, blend);
				break;
			}
		case TextAlignment::Center:
			{
				Vector2 size = Calculate_String_Size(line.c_str(), font_height, final_scale);
				Draw_String(line.c_str(), Vector2((bounds.X + (bounds.Width / 2)) - (size.X / 2), pen_y), font_height, color, Vector2(0.0f, extra_spacing_y), final_scale, blend);
				break;
			}
		case TextAlignment::Right:	
			{
				Vector2 size = Calculate_String_Size(line.c_str(), font_height, final_scale);
				Draw_String(line.c_str(), Vector2(bounds.X + bounds.Width - size.X, pen_y), font_height, color, Vector2(0.0f, extra_spacing_y), final_scale, blend);
				break;
			}		
		case TextAlignment::Justified:
			{
				Vector2 size = Calculate_String_Size(line.c_str(), font_height, final_scale);
				float extra_spacing = (bounds.Width - size.X) / line.length(); // TODO: Utf8 decoding for length.
				Draw_String(line.c_str(), Vector2(bounds.X, pen_y), font_height, color, Vector2(extra_spacing, 0.0f), final_scale, blend);
				break;
			}
		default: DBG_ASSERT_STR(false, "Invalid horizontal alignment.");
		}
		
		pen_y += floorf(((font_height + line_spacing) * final_scale.Y) + extra_spacing_y);
	}
}

Vector2 MarkupFontRenderer::Calculate_String_Size(const char* text, float font_height, Vector2 scale)
{
	bool done = false;
	
	float line_height = m_font_renderer.Calculate_String_Size(" ", font_height, scale).Y;

	// Paaaaaaaarse tags.
	MarkupParser parser;
	parser.Start(text);

	FontRenderState state = m_font_renderer.Begin_Draw(Vector2(0, 0), true);

	while (!done)
	{
		MarkupToken token;
		parser.Next(token);

		switch (token.Type)
		{
		case MarkupTokenType::Text:
			{
				m_font_renderer.Draw(state, token.Value.c_str(), font_height, Color::White, Vector2(0, 0), scale, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);
			}
			break;
		case MarkupTokenType::Tag:
			{
				if (token.Value == "img")
				{
					if (token.Parameters.size() == 3 || token.Parameters.size() == 1 || token.Parameters.size() == 5)
					{
						AtlasFrame* frame = ResourceFactory::Get()->Get_Atlas_Frame(token.Parameters[0].Get_String().c_str());
						if (frame == NULL)
						{
							continue;
						}

						float scale_x = line_height / frame->Rect.Height;
						float scale_y = scale_x;
						float offset_x = 0;
						float offset_y = 0;

						if (token.Parameters.size() >= 3)
						{
							scale_x *= token.Parameters[1].Get_Float();
							scale_y *= token.Parameters[2].Get_Float();
						}

						state.Pen = state.Pen + Vector2(frame->Rect.Width * scale_x, 0.0f);
					}
				}
				else if (token.Value == "key")
				{
					if (token.Parameters.size() == 1 || token.Parameters.size() == 2)
					{	
						const char* output_name = token.Parameters[0].Get_String().c_str();
						OutputBindings::Type output;
						
						InputBindingType::Type type = InputBindingType::Any;
						if (token.Parameters.size() >= 2)
						{							
							const char* key_type = token.Parameters[1].Get_String().c_str();
							if (stricmp(key_type, "joystick") == 0)
							{
								type = InputBindingType::Joystick;
							}
							else if (stricmp(key_type, "keyboard") == 0)
							{
								type = InputBindingType::Keyboard;
							}
						}

						if (OutputBindings::Parse(output_name, output))
						{
							JoystickState* joystick_state = NULL;
							InputBindings::Type input = Input::Get()->Output_To_Active_Input(output, type, &joystick_state);
							std::string atlas_name = "";

							if (InputBindings::ToAtlasFrameName(input, atlas_name, Input::Get()->Get_Active_Joystick_Icon_Set()))
							{
								if (joystick_state && type != InputBindingType::Keyboard)
								{
									std::string result = joystick_state->Get_Custom_Action_Icon(output);
									if (result != "")
									{
										//	DBG_LOG("Atlas=%s", result.c_str());
										atlas_name = result;
									}
								}

								AtlasFrame* frame = ResourceFactory::Get()->Get_Atlas_Frame(atlas_name.c_str());
								if (frame != NULL)
								{
									float key_scale = 1.5f;

									// HACK: Fix steam icons being to large. Should probably downsize the source files really -_-
									if (atlas_name.find("Steam") != std::string::npos)
									{
										key_scale = 1.0f;
									}

									float scaled_line_height = (line_height * key_scale);
									float line_height_diff = scaled_line_height - line_height;
									float scale_x = scaled_line_height / frame->Rect.Height;
									float scale_y = scale_x;

									// We apply a fake scale and vertically center key icons to make them more legible.

									Rect2D box = Rect2D
									(
										state.Pen.X, 
										state.Pen.Y - (line_height_diff * 0.5f),
										frame->Rect.Width * scale_x,
										(frame->Rect.Height * scale_y) 
									);
									state.Pen = state.Pen + Vector2(box.Width, 0.0f);
								}
							}
						}
					}
				}
				else if (token.Value == "avatar")
				{
					// TODO
				}
			}
			break;
		case MarkupTokenType::End:
			{
				done = true;
			}
			break;
		}

		// Calculate maximum size.					
		if (state.Pen.X > state.Max_Size.X)
		{
			state.Max_Size.X = state.Pen.X;
		}
		if (state.Pen.Y > state.Max_Size.Y)
		{
			state.Max_Size.Y = state.Pen.Y;
		}
	}

	Vector2 s = (state.Max_Size - state.Pen_Start);
	if (s.Y == 0)
	{
		s.Y = font_height;
	}

	return s;
}

std::string MarkupFontRenderer::Word_Wrap(const char* text, Rect2D bounds, float font_height, Vector2 scale)
{	
	MarkupParser parser;
	parser.Start(text);

	bool done = false;
	std::string final_text = "";

	// Remove tags from text
	while (!done)
	{
		MarkupToken token;
		parser.Next(token);

		switch (token.Type)
		{
		case MarkupTokenType::Text:
			{
				final_text += token.Value;
			}
			break;
		case MarkupTokenType::Tag:
			{
				// Don't care about tags.
			}
			break;
		case MarkupTokenType::End:
			{
				done = true;
			}
			break;
		}
	}

	std::string wrapped_text = m_font_renderer.Word_Wrap(final_text.c_str(), bounds, font_height, scale);
	std::string output_text = "";

	// Add tags back into text.
	// ARRRRRRRRgh, there is probably a neater way to do this, but I'm being lazy.
	parser = MarkupParser();
	parser.Start(text);

	done = false;

	while (!done)
	{
		MarkupToken token;
		parser.Next(token);

		switch (token.Type)
		{
		case MarkupTokenType::Text:
			{
				std::string match = token.Value;

				while (match.size() > 0 && wrapped_text.size() > 0)
				{
					char match_char = match.at(0);
					char curr_char  = wrapped_text.at(0);

					if (match_char == curr_char || match_char == ' ')
					{
						match = match.substr(1);
					}

					// Escape tag characters.
					if (curr_char == '[')
					{
						output_text += "[";
					}

					wrapped_text = wrapped_text.substr(1);
					output_text += curr_char;
				}
			}
			break;
		case MarkupTokenType::Tag:
			{
				// Don't care about tags.
				output_text += "[" + token.Value;
				if (token.Parameters.size() > 0)
				{
					output_text += "=";

					for (std::vector<Variant>::iterator iter = token.Parameters.begin(); iter != token.Parameters.end(); iter++)
					{
						Variant& var = *iter;

						if (iter != token.Parameters.begin())
						{
							output_text += ",";
						}

						output_text += var.Get_String();
					}
				}
				output_text += "]";
			}
			break;
		case MarkupTokenType::End:
			{
				done = true;
			}
			break;
		}
	}

	return output_text;
}

std::string MarkupFontRenderer::Escape(std::string value)
{
	std::string result;

	for (unsigned int i = 0; i < value.size(); i++)
	{
		char c = value.at(i);
		if (c == '[')
		{
			result += "[";
		}
		result += c;
	}

	return result;
}