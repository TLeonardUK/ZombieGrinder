// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Builder/Compilers/Audio/AudioCompiler.h"
#include "Builder/Builder/AssetBuilder.h"

#include "Engine/Platform/Platform.h"

#include "Engine/IO/StreamFactory.h"

#include "Engine/Audio/Sounds/Sound.h"

#include "Engine/IO/PatchedBinaryStream.h"

#include "libvorbis/include/ogg/ogg.h"
#include "libvorbis/include/vorbis/vorbisenc.h"

struct AudioGroup
{
	std::string name;
	float volume;
};

struct WAV_HEADER
{
    char                RIFF[4];        // RIFF Header      Magic header
    unsigned long       ChunkSize;      // RIFF Chunk Size  
    char                WAVE[4];        // WAVE Header    
};

struct WAV_FMT_CHUNK
{
    char                fmt[4];         // FMT header       
    unsigned long       Subchunk1Size;  // Size of the fmt chunk                                
    unsigned short      AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM 
    unsigned short      NumOfChan;      // Number of channels 1=Mono 2=Sterio                   
    unsigned long       SamplesPerSec;  // Sampling Frequency in Hz                             
    unsigned long       bytesPerSec;    // bytes per second 
    unsigned short      blockAlign;     // 2=16-bit mono, 4=16-bit stereo 
    unsigned short      bitsPerSample;  // Number of bits per sample   
};

#define AUDIO_CONVERSION_BUFFER_SIZE 1024
signed char g_audio_conversion_buffer[AUDIO_CONVERSION_BUFFER_SIZE * 4 + 44]; 

void Seek_To_WAV_Chunk(Stream* stream, const char* chunk_id)
{
	char current_id[5];
	current_id[4] = '\0';

	bool found = false;

	while (!stream->IsEOF())
	{
		for (int i = 0; i < 3; i++)
			current_id[i] = current_id[i + 1];
		current_id[3] = stream->Read<char>();

		if (strcmp(current_id, chunk_id) == 0)
		{
			found = true;
			break;
		}
	}

	DBG_ASSERT(found);
	stream->Seek(stream->Position() - 4);
}

void Normalize_PCM(std::string src, std::string dst, float volume)
{
	Stream* src_stream = StreamFactory::Open(src.c_str(), StreamMode::Read);
	Stream* dst_stream = StreamFactory::Open(dst.c_str(), StreamMode::Write);

	// ----------------------------------------------------------------------------------
	// Header parsing.
	// ----------------------------------------------------------------------------------
	WAV_HEADER header;
	src_stream->ReadBuffer((char*)&header, 0, sizeof(WAV_HEADER));
	DBG_ASSERT(header.RIFF[0] == 'R' && header.RIFF[1] == 'I' && header.RIFF[2] == 'F' && header.RIFF[3] == 'F');
	DBG_ASSERT(header.WAVE[0] == 'W' && header.WAVE[1] == 'A' && header.WAVE[2] == 'V' && header.WAVE[3] == 'E');

	// Seek to the format chunk.
	Seek_To_WAV_Chunk(src_stream, "fmt ");
	WAV_FMT_CHUNK format;
	src_stream->ReadBuffer((char*)&format, 0, sizeof(WAV_FMT_CHUNK));

	DBG_ASSERT(format.fmt[0] == 'f' && format.fmt[1] == 'm' && format.fmt[2] == 't' && format.fmt[3] == ' ');
	DBG_ASSERT(format.AudioFormat == 1);

	int channels = format.NumOfChan;
	int sample_rate = format.SamplesPerSec;
	int sample_depth = format.bitsPerSample;

	// We always leave a little room at the top of the spectrum to make sure we don't end up with clipping issues.
	const float max_volume = 0.9f;

	// Seek to the data chunk.
	Seek_To_WAV_Chunk(src_stream, "data");
	src_stream->ReadBuffer((char*)g_audio_conversion_buffer, 0, 4);
	int pcm_length = src_stream->Read<int>();
	int pcm_start = src_stream->Position();

	// Copy header to destination.
	src_stream->Seek(0);
	while ((int)src_stream->Position() < pcm_start)
	{
		dst_stream->Write<unsigned char>(src_stream->Read<unsigned char>());
	}
	src_stream->Seek(pcm_start);

	// Normalize all audio, volumes should be modified in xml files.
	if (format.bitsPerSample == 8)
	{
		u8 min_val = 0;
		u8 max_val = 0;

		for (int i = 0; i < pcm_length; i++)
		{
			u8 x = src_stream->Read<u8>();
			if (x > max_val)
			{
				max_val = x;
			}
		}

		src_stream->Seek(pcm_start);

		float scale = ((float)255 / (float)Min(abs(min_val), abs(max_val))) * volume * max_volume;

		for (int i = 0; i < pcm_length; i++)
		{
			u8 original = src_stream->Read<u8>();
			u8 output = (u8)(original * scale);
			dst_stream->Write<u8>(output);
		}

		DBG_LOG("Normalized PCM data of '%s' with scale '%.2f' (8bit range=%i/%i)", src.c_str(), scale, min_val, max_val);
	}
	else if (format.bitsPerSample == 16)
	{
		short min_val = 0;
		short max_val = 0;

		for (int i = 0; i < pcm_length/2; i++)
		{
			short x = src_stream->Read<short>();
			if (x > max_val)
			{
				max_val = x;
			}
			if (x < min_val)
			{
				min_val = x;
			}
		}

		src_stream->Seek(pcm_start);

		float scale = ((float)32767 / (float)Min(abs(min_val), abs(max_val))) * volume * max_volume;

		for (int i = 0; i < pcm_length/2; i++)
		{
			short original = src_stream->Read<short>();
			short output = (short)(original * scale);
			dst_stream->Write<short>(output);
		}

		DBG_LOG("Normalized PCM data of '%s' with scale '%.2f' (16bit range=%i/%i)", src.c_str(), scale, min_val, max_val);
	}
	else
	{
		DBG_ASSERT_STR(false, "[ERROR] Could not normalize PCM data of '%s' unknown bitsPerSample=%i, channels=%i.", src.c_str(), format.bitsPerSample, format.NumOfChan);
	}

	// Copy the rest.
	while (src_stream->Bytes_Remaining() > 0)
	{
		dst_stream->Write<unsigned char>(src_stream->Read<unsigned char>());
	}

	SAFE_DELETE(src_stream);
	SAFE_DELETE(dst_stream);
}

// Based on encoder_example.c, hopefully this is right.
void AudioCompiler::Convert_To_OGG(std::string src, std::string dst)
{
	ogg_stream_state os; /* take physical pages, weld into a logical
						stream of packets */
	ogg_page         og; /* one Ogg bitstream page.  Vorbis packets are inside */
	ogg_packet       op; /* one raw packet of data for decode */
	vorbis_info      vi; /* struct that stores all the static vorbis bitstream
						settings */
	vorbis_comment   vc; /* struct that stores all the user comments */
	vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
	vorbis_block     vb; /* local working space for packet->PCM decode */
	
	int eos = 0, ret;

	Stream* src_stream = StreamFactory::Open(src.c_str(), StreamMode::Read);
	Stream* dst_stream = StreamFactory::Open(dst.c_str(), StreamMode::Write);
	
	// ----------------------------------------------------------------------------------
	// Header parsing.
	// ----------------------------------------------------------------------------------
	WAV_HEADER header;
	src_stream->ReadBuffer((char*)&header, 0, sizeof(WAV_HEADER));
	DBG_ASSERT(header.RIFF[0] == 'R' && header.RIFF[1] == 'I' && header.RIFF[2] == 'F' && header.RIFF[3] == 'F');
	DBG_ASSERT(header.WAVE[0] == 'W' && header.WAVE[1] == 'A' && header.WAVE[2] == 'V' && header.WAVE[3] == 'E');

	// Seek to the format chunk.
	Seek_To_WAV_Chunk(src_stream, "fmt ");
	WAV_FMT_CHUNK format;
	src_stream->ReadBuffer((char*)&format, 0, sizeof(WAV_FMT_CHUNK));
	
	DBG_ASSERT(format.fmt[0] == 'f' && format.fmt[1] == 'm' && format.fmt[2] == 't' && format.fmt[3] == ' ');
	DBG_ASSERT(format.AudioFormat == 1);

	int channels		= format.NumOfChan;
	int sample_rate		= format.SamplesPerSec;
	int sample_depth	= format.bitsPerSample;

	// Seek to the data chunk.
	Seek_To_WAV_Chunk(src_stream, "data");
	src_stream->ReadBuffer((char*)g_audio_conversion_buffer, 0, 8);

	// ----------------------------------------------------------------------------------
	// OGG Encoding.
	// ----------------------------------------------------------------------------------
	vorbis_info_init(&vi);

	ret = vorbis_encode_init_vbr(&vi, channels, sample_rate, AUDIO_CONVERSION_QUALITY);
	DBG_ASSERT(ret == 0);

	vorbis_comment_init(&vc);
	vorbis_comment_add_tag(&vc, "ENCODER", "XGE Data Builder");

	vorbis_analysis_init(&vd, &vi);
	vorbis_block_init(&vd, &vb);

	ogg_stream_init(&os, rand());
	
	{
		ogg_packet header;
		ogg_packet header_comm;
		ogg_packet header_code;
	
		vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
		ogg_stream_packetin(&os, &header); 
		ogg_stream_packetin(&os, &header_comm);
		ogg_stream_packetin(&os, &header_code);

		while (!eos)
		{
			int result = ogg_stream_flush(&os,&og);
			if (result == 0) break;

			dst_stream->WriteBuffer((char*)og.header, 0, og.header_len);
			dst_stream->WriteBuffer((char*)og.body, 0, og.body_len);
		}
	}

	while (!eos)
	{
		long i;
		int bytes_per_sample = 2 * channels;
		long bytes = Min((int)src_stream->Bytes_Remaining(), AUDIO_CONVERSION_BUFFER_SIZE * bytes_per_sample);

		src_stream->ReadBuffer((char*)g_audio_conversion_buffer, 0, bytes);

		if (bytes == 0)
		{
			vorbis_analysis_wrote(&vd, 0);
		}
		else
		{
			float **buffer = vorbis_analysis_buffer(&vd, AUDIO_CONVERSION_BUFFER_SIZE);

			for (i = 0; i < bytes / bytes_per_sample; i++)
			{
				buffer[0][i] = ((g_audio_conversion_buffer[i*bytes_per_sample+1]<<8)|(0x00ff&(int)g_audio_conversion_buffer[i*bytes_per_sample]))/32768.f;
				if (channels > 1)
				{
					buffer[1][i] = ((g_audio_conversion_buffer[i*bytes_per_sample+3]<<8)|(0x00ff&(int)g_audio_conversion_buffer[i*bytes_per_sample+2]))/32768.f;
				}
			}

			vorbis_analysis_wrote(&vd, i);
		}

		while (vorbis_analysis_blockout(&vd, &vb) == 1)
		{
			vorbis_analysis(&vb, NULL);
			vorbis_bitrate_addblock(&vb);

			while (vorbis_bitrate_flushpacket(&vd, &op))
			{
				ogg_stream_packetin(&os, &op);

				while (!eos)
				{
					int result = ogg_stream_pageout(&os, &og);
					if (result == 0)
						break;
					
					dst_stream->WriteBuffer((char*)og.header, 0, og.header_len);
					dst_stream->WriteBuffer((char*)og.body, 0, og.body_len);

					if (ogg_page_eos(&og))
						eos = 1;
				}
			}
		}
	}

	ogg_stream_clear(&os);
	vorbis_block_clear(&vb);
	vorbis_dsp_clear(&vd);
	vorbis_comment_clear(&vc);
	vorbis_info_clear(&vi);

	SAFE_DELETE(dst_stream);
	SAFE_DELETE(src_stream);
}

int AudioCompiler::Get_Version()
{
	return 48;
}

std::vector<AssetToCompile> AudioCompiler::Gather_Assets(AssetBuilder* builder)
{
	std::vector<AssetToCompile> result;

	ConfigFile* file = builder->Get_Package_Config_File();

	if (!file->Contains("audio_banks"))
	{
		return result;
	}

	std::vector<ConfigFileNode> atlases	= file->Get<std::vector<ConfigFileNode> >("audio_banks/bank");

	// Load audio banks.
	for (std::vector<ConfigFileNode>::iterator atlas_iter = atlases.begin(); atlas_iter != atlases.end(); atlas_iter++)
	{
		ConfigFileNode& node = *atlas_iter;

		AssetToCompile asset;
		asset.Source_Path = file->Get<const char*>("file", node, true);
		asset.Priority = "";

		if (file->Contains("priority", node, true))
		{
			asset.Priority = file->Get<const char*>("priority", node, true);
		}

		// Calculate all dependencies.
		std::string resolved_asset_path = builder->Resolve_Asset_Path(asset.Source_Path);

		ConfigFile asset_file;
		if (!asset_file.Load(resolved_asset_path.c_str()))
		{
			DBG_LOG("[WARNING] Failed to check dependencies for atlas. Bank file does not exist! '%s'.", asset.Source_Path.c_str());
			continue;
		}

		std::vector<ConfigFileNode> images = asset_file.Get<std::vector<ConfigFileNode> >("sounds/sound");
		for (std::vector<ConfigFileNode>::iterator image_iter = images.begin(); image_iter != images.end(); image_iter++)
		{
			ConfigFileNode& image_node = *image_iter;

			if (asset_file.Contains("file", image_node, true))
			{
				std::string image_path = asset_file.Get<const char*>("file", image_node, true);
				asset.Dependent_Files.push_back(image_path);
			}
		}

		result.push_back(asset);
	}

	return result;
}

bool AudioCompiler::Build_Asset(AssetBuilder* builder, AssetToCompile asset)
{
	// Calculate all dependencies.
	std::string resolved_asset_path = builder->Resolve_Asset_Path(asset.Source_Path);
	std::string compiled_asset_path = builder->Get_Compiled_Asset_File(asset.Source_Path);

	// Load asset file.
	ConfigFile asset_file;
	if (!asset_file.Load(resolved_asset_path.c_str()))
	{
		return false;
	}

	const char* bank_name = asset_file.Get<const char*>("settings/name", NULL, false);

	// Load groups.
	std::vector<AudioGroup> groups;

	if (asset_file.Contains("groups"))
	{
		std::vector<ConfigFileNode> group_nodes = asset_file.Get<std::vector<ConfigFileNode> >("groups/group");
		for (std::vector<ConfigFileNode>::iterator iter = group_nodes.begin(); iter != group_nodes.end(); iter++)
		{
			ConfigFileNode& node = *iter;

			AudioGroup group;
			group.name	 = asset_file.Get<const char*>("name", node, true);
			group.volume = asset_file.Get<float>("volume", node, true);

			groups.push_back(group);
		}
	}

	// Compile each of the sounds seperately.
	int offset = 0;
	std::vector<ConfigFileNode> sounds = asset_file.Get<std::vector<ConfigFileNode> >("sounds/sound");
	for (std::vector<ConfigFileNode>::iterator iter = sounds.begin(); iter != sounds.end(); iter++)
	{
		ConfigFileNode& node = *iter;

		const char* name = asset_file.Get<const char*>("name", node, true);
		const char* sound_file = asset_file.Get<const char*>("file", node, true);
		const char* group_name = asset_file.Contains("group", node, true) ? asset_file.Get<const char*>("group", node, true) : "";

		float volume = 1.0f;

		for (std::vector<AudioGroup>::iterator group_iter = groups.begin(); group_iter != groups.end(); group_iter++)
		{
			AudioGroup& group = *group_iter;
			if (group.name == group_name)
			{
				volume = group.volume;
				break;
			}
		}

		SoundFlags::Type flags = SoundFlags::NONE;
		SoundFormat::Type format = SoundFormat::WAV;

		bool looped = asset_file.Get<bool>("looping", node, true);
		if (looped == true)
			flags = (SoundFlags::Type)((int)flags | (int)SoundFlags::Loop);

		bool streaming = asset_file.Get<bool>("streaming", node, true);
		if (streaming == true)
			flags = (SoundFlags::Type)((int)flags | (int)SoundFlags::Streaming);

		std::string extension = Platform::Get()->Extract_Extension(sound_file);
		if (extension == "wav")
		{
			format = SoundFormat::WAV;
		}
		else if (extension == "ogg")
		{
			format = SoundFormat::OGG;			
		}
		else if (extension == "it")
		{
			format = SoundFormat::IT;
		}

		std::string resolved_sound_path = builder->Resolve_Asset_Path(sound_file);
		std::string compiled_sound_path = builder->Get_Compiled_Asset_File(sound_file);
		std::string normalized_sound_path = builder->Get_Compiled_Asset_File(sound_file) + ".normalized";

		DBG_LOG("Compiling file resource '%s' to '%s'.", sound_file, compiled_sound_path.c_str());

		// Write out compiled version.
		PackageFile* file = builder->Get_Package_File();
		PatchedBinaryStream* stream = new PatchedBinaryStream(compiled_sound_path.c_str(), StreamMode::Write);

		// Normalize all wav files.
		if (format == SoundFormat::WAV)
		{
			Normalize_PCM(resolved_sound_path.c_str(), normalized_sound_path.c_str(), volume);
		}
		else
		{
			Platform::Get()->Copy_File(resolved_sound_path.c_str(), normalized_sound_path.c_str());
		}		

		Stream* sound_stream = StreamFactory::Open(normalized_sound_path.c_str(), StreamMode::Read);

		// Do conversion to OGG rather than using raw PCM data.
		if (format == SoundFormat::WAV && sound_stream->Length() > ogg_conversion_threshold)
		{
			int original_size = sound_stream->Length();
			SAFE_DELETE(sound_stream);			

			std::string ogg_path = compiled_sound_path + ".ogg";
			Convert_To_OGG(normalized_sound_path, ogg_path);
			format = SoundFormat::OGG;

			sound_stream = StreamFactory::Open(ogg_path.c_str(), StreamMode::Read);			
			int new_size = sound_stream->Length();

			//DBG_LOG("Converted '%s' to OGG with %.2f x compression rate.", resolved_sound_path.c_str(), (float)original_size / (float)new_size);
		}

		// CompiledAudioSound
		int name_ptr_index = stream->Create_Pointer();
		stream->Write<u32>(StringHelper::Hash(name));
		stream->Write<u32>(flags);
		stream->Write<u32>(format);
		int data_ptr_index = stream->Create_Pointer();
		stream->Write<u32>(sound_stream->Length());

		// Write in strings.
		stream->Patch_Pointer(name_ptr_index);
		stream->WriteNullTerminatedString(name);

		// Write in audio data.
		stream->Patch_Pointer(data_ptr_index);
		sound_stream->CopyTo(stream);

		sound_stream->Close();
		SAFE_DELETE(sound_stream);

		stream->Close();
		SAFE_DELETE(stream);
	}

	return true;
}

void AudioCompiler::Add_Chunks(AssetBuilder* builder, AssetToCompile asset)
{
	for (std::vector<std::string>::iterator iter = asset.Dependent_Files.begin(); iter != asset.Dependent_Files.end(); iter++)
	{
		std::string resolved_asset_path = builder->Resolve_Asset_Path(*iter);
		std::string compiled_asset_path = builder->Get_Compiled_Asset_File(*iter);

		builder->Get_Package_File()->Add_Chunk(compiled_asset_path.c_str(), (*iter).c_str(), PackageFileChunkType::Sound, true, asset.Priority.c_str(), asset.Out_Of_Date);
	}
}