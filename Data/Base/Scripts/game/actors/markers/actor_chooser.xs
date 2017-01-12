// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;

[
	Placeable(true), 
	Name("Actor Chooser"), 
	Description("Despawns all but n amount of the actors linked to this actor.") 
]
public class Actor_Chooser : Actor
{
	[ 
		Name("Count"),
		Description("Number of actors to choose.")
	]
	public serialized(150) int Count = 1;
	
	Actor_Chooser()
	{
		Bounding_Box = Vec4(0, 0, 16, 16);
	}

	default state Idle
	{
		event void On_Tick()
		{
			Actor[] tags = Scene.Find_Actors_By_Tag(this.Link);
			if (tags.Length() > Count)
			{
				int to_remove = tags.Length() - Count;
				Log.Write("Actor chooser is removing "+to_remove+"/"+tags.Length()+" entries (with tag '"+this.Link+"').");

				for (int i = 0; i < to_remove; i++)
				{
					int remove_index = Math.Rand(0, tags.Length());

					Actor tagged_actor = tags[remove_index];
					Scene.Despawn(tagged_actor);

					Log.Write(" - Removed actor "+tagged_actor.Net_ID+".");

					tags.RemoveIndex(remove_index);
				}
			}
			Scene.Despawn(this);
		}
	}

	components
	{
		SpriteComponent sprite
		{
			Frame 			= "actor_actor_chooser_default_0";
			Color 			= Vec4(255.0, 255.0, 255.0, 255.0);
			Visible 		= false; 
			Editor_Visible 	= true;
		}
	}
}
