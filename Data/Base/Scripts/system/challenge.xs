// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

public enum ChallengeTimeframe
{ 
    Daily,
    Weekly,
    Monthly
}

[
	Name("Challenge"), 
	Description("Dailog challenge base class!") 
]
public native("Challenge") class Challenge
{	
    static Challenge_Item_Drop_Table        m_item_drop_Table       = new Challenge_Item_Drop_Table();
    static Challenge_XP_Drop_Table          m_xp_drop_Table         = new Challenge_XP_Drop_Table();
    static Challenge_Coin_Drop_Table        m_coin_drop_Table       = new Challenge_Coin_Drop_Table();
    static Challenge_Rare_Item_Drop_Table   m_rare_item_drop_Table  = new Challenge_Rare_Item_Drop_Table();
    static Challenge_Rare_XP_Drop_Table     m_rare_xp_drop_Table    = new Challenge_Rare_XP_Drop_Table();
    static Challenge_Rare_Coin_Drop_Table   m_rare_coin_drop_Table  = new Challenge_Rare_Coin_Drop_Table();

	property string ID
	{
		native("Get_ID") string Get();
		native("Set_ID") void Set(string value);		
	}

	property string Name
	{
		native("Get_Name") string Get();
		native("Set_Name") void Set(string value);	
	}

	property string Description
	{
		native("Get_Description") string Get();
		native("Set_Description") void Set(string value);		
	}

	property float Max_Progress
	{
		native("Get_Max_Progress") float Get();
		native("Set_Max_Progress") void Set(float value);		
	}

	property float Progress
	{
		native("Get_Progress") float Get();
		native("Set_Progress") void Set(float value);		
	}

	property ChallengeTimeframe Timeframe
	{
		native("Get_Timeframe") ChallengeTimeframe Get();
		native("Set_Timeframe") void Set(ChallengeTimeframe value);		
	}

	property bool Requires_Activation
	{
		native("Get_Requires_Activation") bool Get();
		native("Set_Requires_Activation") void Set(bool value);		
	}

	property bool In_Custom_Map
	{
		native("Get_In_Custom_Map") bool Get();
	}
	
	property bool Specific_Date
	{
		native("Get_Specific_Date") bool Get();
		native("Set_Specific_Date") void Set(bool value);		
	}
	
	property int Start_Day
	{
		native("Get_Start_Day") int Get();
		native("Set_Start_Day") void Set(int value);		
	}
	
	property int Start_Month
	{
		native("Get_Start_Month") int Get();
		native("Set_Start_Month") void Set(int value);		
	}
	
	property bool Announce
	{
		native("Get_Announce") bool Get();
		native("Set_Announce") void Set(bool value);		
	}
	property string Announce_Image
	{
		native("Get_Announce_Image") string Get();
		native("Set_Announce_Image") void Set(string value);		
	}
	property string Announce_Name
	{
		native("Get_Announce_Name") string Get();
		native("Set_Announce_Name") void Set(string value);		
	}
	property string Announce_Description
	{
		native("Get_Announce_Description") string Get();
		native("Set_Announce_Description") void Set(string value);		
	}
	
	public native("Start_Custom_Map") void Start_Custom_Map(string map, int seed);
	public native("Was_Completed") static bool Was_Completed(ChallengeTimeframe timeframe);
	
    public virtual Item_Archetype[] Get_Rewards()
    { 
        return Get_Standard_Rewards(1, 2);
    }

    public Item_Archetype[] Get_Standard_Rewards(int min, int max)
    {
        // Default to standard drop tables.
        Drop_Table coin_table = m_coin_drop_Table;
        Drop_Table xp_table = m_xp_drop_Table;
        Drop_Table item_table = m_item_drop_Table;

        // Weekly and monthly pick from the rare list.
        if (Timeframe == ChallengeTimeframe.Weekly || 
            Timeframe == ChallengeTimeframe.Monthly)
        {
            coin_table = m_rare_coin_drop_Table;
            xp_table = m_rare_xp_drop_Table;
            item_table = m_rare_item_drop_Table;
        }

        // Random number of rewards.
        int count = Math.Rand(min, max + 1);
        Item_Archetype[] rewards = new Item_Archetype[count];    
        for (int i = 0; i < count; i++)
        {
            int idx = Math.Rand(0, 3);
            if (i == 0 || idx > 1)
            {            
                rewards[i] = Item_Archetype.Find_By_Type(item_table.Get_Drop());
            }    
            else if (idx == 0)
            {
                rewards[i] = Item_Archetype.Find_By_Type(coin_table.Get_Drop());
            }
            else if (idx == 1)
            {
                rewards[i] = Item_Archetype.Find_By_Type(xp_table.Get_Drop());
            }
        }
        return rewards;
    }
}
 

