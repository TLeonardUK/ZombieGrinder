// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontRenderer.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Renderer/RenderBatch.h"

#include "Engine/Renderer/Textures/Texture.h"
#include "Engine/Renderer/Textures/TextureFactory.h"

#include "Engine/Renderer/PrimitiveRenderer.h"

#include "Engine/Renderer/Text/TagReplacer.h"

#include "Engine/Scene/Camera.h"

#include "Generic/Helper/StringHelper.h"

#include "Generic/Math/Math.h"
#include "Generic/Types/UTF8String.h"

#include "Engine/Profiling/ProfilingManager.h"

#include <float.h>

// TODO: Buffered rendering.

FontRenderer::FontRenderer()
	: m_buffered(NULL)
	, m_font(NULL)
	, m_buffer_target(NULL)
	, m_buffer_texture(NULL)
	, m_buffer_text(NULL)
	, m_buffer_text_length(0)
	, m_shadow(true)
{
}

FontRenderer::FontRenderer(FontHandle* font, bool buffered, bool shadow)
	: m_buffered(buffered)
	, m_font(font)
	, m_buffer_target(NULL)
	, m_buffer_texture(NULL)
	, m_buffer_text(NULL)
	, m_buffer_text_length(0)
	, m_shadow(shadow)
{
}

FontRenderer::~FontRenderer()
{
	SAFE_DELETE(m_buffer_target);
	SAFE_DELETE(m_buffer_texture);
	SAFE_DELETE(m_buffer_text);
}

FontRenderState FontRenderer::Begin_Draw(Vector2 pen, bool do_not_draw)
{
	FontRenderState state;

	// Round location, sub-pixel fonts look crap.
	pen.X = floorf(pen.X);
	pen.Y = floorf(pen.Y);

	state.DoNotDraw = do_not_draw;
	state.Pen_Start = pen;
	state.Pen = pen;
	state.Max_Size = Vector2(0, 0);

	return state;
}

void FontRenderer::Draw(FontRenderState& state, const char* inText, float font_height, Color color, Vector2 extra_spacing, Vector2 output_scale, RendererOption::Type blend)
{
	PROFILE_FUNCTION();

	// Apply scaling to size.
	font_height *= output_scale.Y;

	float line_spacing = font_height * 0.2f;
	float font_scale = font_height / m_font->Get()->Get_Base_Height();

	// Grab general settings.
	Font*			font		= m_font->Get();
	RenderPipeline*	pipeline	= RenderPipeline::Get();
	RenderBatch*	batch		= NULL;
	
	// Do tag replacement.
	UTF8String text = TagReplacer::Replace(inText);

	float shadow_scale = font->Get_Shadow_Scale();

	// Draw each glyph!
	Texture* glyph_texture = NULL;
	int length = text.Length(); 
	
	int pass = 0;
	if (m_shadow == false)
		pass = 1;

	float start_pen_x = state.Pen.X;
	float start_pen_y = state.Pen.Y;
	float start_max_x = state.Max_Size.X;
	float start_max_y = state.Max_Size.Y;

	for (; pass < 2; pass++)
	{
		state.Pen.X = start_pen_x;
		state.Pen.Y = start_pen_y;
		state.Max_Size.X = start_max_x;
		state.Max_Size.Y = start_max_y;

		UTF8String::UTF8Iterator iter(text.C_Str());

		for (int i = 0; i < length; i++)
		{
			u32	glyph = 0;
			DBG_ASSERT(iter.Advance(glyph));

			FontGlyph font_glyph = font->Get_Glyph(glyph);

			// Newline?
			if (glyph == '\n')
			{
				state.Pen.Y += floorf(font_height + line_spacing + extra_spacing.Y);
				state.Pen.X = state.Pen_Start.X;
				continue;
			}

			// Skip certain whitespace that we don't want to render.
			if (glyph == '\r')
			{
				continue;
			}

			// Bind font glyph.
			if (font_glyph.TexturePtr != glyph_texture && 
				font_glyph.TexturePtr != NULL && 
				state.DoNotDraw == false)
			{
				batch = pipeline->Get_Render_Batch(font_glyph.TexturePtr, PrimitiveType::Quad);
				batch->Set_Alpha_Test		(true);
				batch->Set_Blend			(true);
				batch->Set_Blend_Function	(blend);
				batch->Set_Color			(color);
				glyph_texture = font_glyph.TexturePtr;
			}
			
			// Calculate position of glyph.
			Rect2D uv = font_glyph.UV;
			float scaled_advance = (font_glyph.Advance * font_scale);

			Rect2D rect = Rect2D(
				state.Pen.X,
				state.Pen.Y,
				font_glyph.Size.X * font_scale,
				font_glyph.Size.Y * font_scale
			);
			
			// Calculate size.
			if (rect.X + scaled_advance > state.Max_Size.X)
			{
				state.Max_Size.X = rect.X + scaled_advance;
			}
			if (rect.Y + rect.Height > state.Max_Size.Y)
			{
				state.Max_Size.Y = rect.Y + rect.Height;
			}

			// Draw!
			if (batch != NULL && state.DoNotDraw == false)
			{	
				if (pass == 0)
				{
					batch->Set_Color(Color(0, 0, 0, color.A));

 					int offset = 1;
					float shadow_offset_scale_x = font_scale * shadow_scale;
					float shadow_offset_scale_y = font_scale * shadow_scale;

					for (int x = -1; x <= 1; x++)
					{
						for (int y = -1; y <= 1; y++)
						{
							if (x == 0 && y == 0)
							{
								continue;
							}
							batch->Draw_Quad(Rect2D(rect.X + (x * shadow_offset_scale_x), rect.Y + (y * shadow_offset_scale_y), rect.Width, rect.Height), uv, 0.0f);
						}
					}
					
					batch->Set_Color(color);
				}
				else
				{
					batch->Draw_Quad(rect, uv, 0.0f);
				}
			}

			// Advance the char offset.
			state.Pen.X += scaled_advance + extra_spacing.X;
		}
	}
}

void FontRenderer::Draw_String(const char* text, Vector2 location, float font_height, Color color, Vector2 extra_spacing, Vector2 output_scale, RendererOption::Type blend)
{
	// Do tag replacement.
	std::string replaced = TagReplacer::Replace(text);
	text = replaced.c_str();

	// Draw the text.
	FontRenderState state = Begin_Draw(location);
	Draw(state, text, font_height, color, Vector2(0, 0), output_scale, blend);
}

void FontRenderer::Draw_String(const char* text, Rect2D bounds, float font_height, Color color, TextAlignment::Type horizontal_align, TextAlignment::Type vertical_align, Vector2 output_scale, RendererOption::Type blend)
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

	//PrimitiveRenderer pr;
	//pr.Draw_Solid_Quad(bounds, Color::Red);
	
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

Vector2 FontRenderer::Calculate_String_Size(const char* text, float font_height, Vector2 scale)
{
	FontRenderState state = Begin_Draw(Vector2(0, 0), true);
	Draw(state, text, font_height, Color::White, Vector2(0, 0), scale, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);
	return (state.Max_Size - state.Pen_Start);
}
	
std::string FontRenderer::Word_Wrap(const char* inText, Rect2D bounds, float font_height, Vector2 scale)
{
	PROFILE_FUNCTION();

	int last_safe_split_point = -1;
	float last_safe_split_point_line_width = 0.0f;

	UTF8String result;
	UTF8String text = inText;
	int len = text.Length();

	Font* font = m_font->Get();
	float line_width = 0.0f;

	int result_len = 0;

	// Apply scaling to size.
	font_height *= scale.Y;

	float line_spacing = font_height * 0.2f;
	float font_scale = font_height / m_font->Get()->Get_Base_Height();

	for (int i = 0; i < len; i++)
	{
		UTF8String c = text[i];
		u32 char_code = c.GetCharCode(0);
		float start_line_width = line_width;

		// Calculate character size.
		FontGlyph font_glyph = font->Get_Glyph(char_code);

		// Calculate position of glyph.
		float scaled_advance = (font_glyph.Advance * font_scale);

		if (char_code != '\n' && char_code != '\r')
		{
			line_width += scaled_advance;
		}

		// Over max width? Split.
		if (line_width > bounds.Width)
		{
			// If we are in the middle of a word, go back till we find the last space.
			if (last_safe_split_point >= 0)
			{
				result = result.Replace(last_safe_split_point, 1, "\n"); 
				result += c;
				result_len++;
				last_safe_split_point = -1;
				line_width = (line_width - last_safe_split_point_line_width);
			}
			else
			{
				result += "\n";
				result += c;
				result_len += 2;
				last_safe_split_point = -1;
				line_width = (line_width - start_line_width);
			}
		}
		// Just append to output.
		else
		{
			result += c;
			result_len++;

			// Newline?
			if (char_code == '\n' || char_code == '\r')
			{
				line_width = 0.0f;
				last_safe_split_point = -1;
			}

			// Keep track of best place to split.
			else if (char_code == ' ')
			{
				last_safe_split_point = result_len - 1;
				last_safe_split_point_line_width = line_width;
			}
		}
	}

	return result.C_Str();
}
	
