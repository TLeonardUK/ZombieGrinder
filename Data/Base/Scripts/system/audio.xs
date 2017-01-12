// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//	Handle for tracking and updating a piece of audio.
// -----------------------------------------------------------------------------
public static native("Audio_Channel") class Audio_Channel
{
	property Vec3 Position
	{	
		public native("Get_Position") Vec3 Get();
		public native("Set_Position") void Set(Vec3 val);
	}
	
	property float Progress
	{	
		public native("Get_Progress") float Get();
		public native("Set_Progress") void Set(float val);
	}
	
	property float Pan
	{	
		public native("Get_Pan") float Get();
		public native("Set_Pan") void Set(float val);
	}

	property float Rate
	{	
		public native("Get_Rate") float Get();
		public native("Set_Rate") void Set(float val);
	}
	
	property float Volume
	{	
		public native("Get_Volume") float Get();
		public native("Set_Volume") void Set(float val);
	}	
	
	property bool Is_Paused
	{	
		public native("Is_Paused") bool Get();
	}	
	
	property bool Is_Playing
	{	
		public native("Is_Playing") bool Get();
	}	
	
	property bool Is_Muted
	{	
		public native("Is_Muted") bool Get();
	}	
	
	public native("Stop") void Stop();
	
	public native("Pause") void Pause();
	public native("Resume") void Resume();

	public native("Mute") void Mute();
	public native("Unmute") void Unmute();
}

// -----------------------------------------------------------------------------
//	Audio stuff
// -----------------------------------------------------------------------------
public static native("Audio") class Audio
{
	property float BGM_Volume_Multiplier
	{	
		public static native("Get_BGM_Volume_Multiplier") float Get();
		public static native("Set_BGM_Volume_Multiplier") void Set(float val);
	}	

	// Plays a sound at a given spatial position.
	public static native("Play_Spatial") Audio_Channel Play3D(string resource, Vec3 position, bool start_paused = false);
	
	// Plays a sound globally without spatial falloff.
	public static native("Play_Global") Audio_Channel Play2D(string resource, bool start_paused = false);	

	// Changes the currently playing BGM.
	public static native("Play_BGM") void PlayBGM(string resource);		
	public static native("Push_BGM") void Push_BGM(string resource);		
	public static native("Pop_BGM") void Pop_BGM();		

	// Set listener position.
	public static native("Set_Listener_Position") void SetListenerPosition(int listener_index, Vec3 position);
}

 