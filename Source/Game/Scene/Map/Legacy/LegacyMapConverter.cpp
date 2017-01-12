// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Map/Legacy/LegacyMapConverter.h"
#include "Engine/Scene/Map/Map.h"
#include "Engine/Scene/Map/MapFile.h"
#include "Game/Scene/Map/Blocks/MapFileLayerBlock.h"
#include "Game/Scene/Map/Blocks/MapFileObjectBlock.h"
#include "Engine/Scene/Map/Blocks/MapFileHeaderBlock.h"
#include "Game/Scene/Actors/CompositeActor.h"
#include "Game/Scene/Actors/ScriptedActor.h"
#include "Game/Scene/Actors/Components/Drawable/SpriteComponent.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/Engine/GameEngine.h"

#include "Game/Runner/Game.h"

#include "Generic/Types/Rect2D.h"

#include "Engine/IO/StreamFactory.h"
#include "Engine/IO/Stream.h"

#include "Engine/Renderer/Atlases/AtlasFactory.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"

LegacyMapConverter::LegacyMapConverter()
{
		// Tileset mappings.
#define MAP(x, y) m_tileset_map.Set(StringHelper::Hash(x), y);
		MAP("pak::bin\\config\\tilesets\\default.ini",			"Main");
		MAP("pak::bin\\config\\tilesets\\tilesyes.ini",			"Main");
		MAP("pak::bin\\config\\tilesets\\random_dungeon.ini",	"Random");
		MAP("pak::bin\\config\\tilesets\\lab.ini",				"Lab");
		MAP("pak::bin\\config\\tilesets\\sewer.ini",			"Sewer");
		MAP("pak::bin\\config\\tilesets\\egypt_tiles.ini",		"Egypt");
		MAP("pak::bin\\config\\tilesets\\heist.ini",			"Heist");
#undef MAP

		// Image mappings.
#define IMG(url, atlas_file, atlas_frame) m_image_map.Set(StringHelper::Hash(url), atlas_frame);
	IMG("pak::bin\\graphics\\achievements\\001_zombinator.png",                               "Data/Atlases/System.xml",                        "achievement_001_zombinator")
	IMG("pak::bin\\graphics\\achievements\\002_failure.png",                                  "Data/Atlases/System.xml",                        "achievement_002_failure")
	IMG("pak::bin\\graphics\\achievements\\003_saviour.png",                                  "Data/Atlases/System.xml",                        "achievement_003_saviour")
	IMG("pak::bin\\graphics\\achievements\\004_fbichecklist.png",                             "Data/Atlases/System.xml",                        "achievement_004_fbichecklist")
	IMG("pak::bin\\graphics\\achievements\\005_headshothoncho.png",                           "Data/Atlases/System.xml",                        "achievement_005_headshothoncho")
	IMG("pak::bin\\graphics\\achievements\\006_mammy.png",                                    "Data/Atlases/System.xml",                        "achievement_006_mammy")
	IMG("pak::bin\\graphics\\achievements\\007_penetrator.png",                               "Data/Atlases/System.xml",                        "achievement_007_penetrator")
	IMG("pak::bin\\graphics\\achievements\\008_n00b.png",                                     "Data/Atlases/System.xml",                        "achievement_008_n00b")
	IMG("pak::bin\\graphics\\achievements\\009_dick.png",                                     "Data/Atlases/System.xml",                        "achievement_009_dick")
	IMG("pak::bin\\graphics\\achievements\\010_marathonman.png",                              "Data/Atlases/System.xml",                        "achievement_010_marathonman")
	IMG("pak::bin\\graphics\\achievements\\011_cremationship.png",                            "Data/Atlases/System.xml",                        "achievement_011_cremationship")
	IMG("pak::bin\\graphics\\achievements\\012_frigid.png",                                   "Data/Atlases/System.xml",                        "achievement_012_frigid")
	IMG("pak::bin\\graphics\\achievements\\013_redmist.png",                                  "Data/Atlases/System.xml",                        "achievement_013_redmist")
	IMG("pak::bin\\graphics\\achievements\\014_daisychain.png",                               "Data/Atlases/System.xml",                        "achievement_014_daisychain")
	IMG("pak::bin\\graphics\\achievements\\015_bronze.png",                                   "Data/Atlases/System.xml",                        "achievement_015_bronze")
	IMG("pak::bin\\graphics\\achievements\\016_silver.png",                                   "Data/Atlases/System.xml",                        "achievement_016_silver")
	IMG("pak::bin\\graphics\\achievements\\017_gold.png",                                     "Data/Atlases/System.xml",                        "achievement_017_gold")
	IMG("pak::bin\\graphics\\achievements\\018_iceandfire.png",                               "Data/Atlases/System.xml",                        "achievement_018_iceandfire")
	IMG("pak::bin\\graphics\\achievements\\019_zombiekiller.png",                             "Data/Atlases/System.xml",                        "achievement_019_zombiekiller")
	IMG("pak::bin\\graphics\\achievements\\020_zombiemaster.png",                             "Data/Atlases/System.xml",                        "achievement_020_zombiemaster")
	IMG("pak::bin\\graphics\\achievements\\021_zombiegrinder.png",                            "Data/Atlases/System.xml",                        "achievement_021_zombiegrinder")
	IMG("pak::bin\\graphics\\achievements\\022_eagleeye.png",                                 "Data/Atlases/System.xml",                        "achievement_022_eagleeye")
	IMG("pak::bin\\graphics\\achievements\\023_oniomania.png",                                "Data/Atlases/System.xml",                        "achievement_023_oniomania")
	IMG("pak::bin\\graphics\\achievements\\024_ihateboxes.png",                               "Data/Atlases/System.xml",                        "achievement_024_ihateboxes")
	IMG("pak::bin\\graphics\\achievements\\025_trickshot.png",                                "Data/Atlases/System.xml",                        "achievement_025_trickshot")
	IMG("pak::bin\\graphics\\achievements\\026_youdidntcrash.png",                            "Data/Atlases/System.xml",                        "achievement_026_youdidntcrash")
	IMG("pak::bin\\graphics\\achievements\\027_rapist.png",                                   "Data/Atlases/System.xml",                        "achievement_027_rapist")
	IMG("pak::bin\\graphics\\achievements\\028_cheater.png",                                  "Data/Atlases/System.xml",                        "achievement_028_cheater")
	IMG("pak::bin\\graphics\\achievements\\029_canttouchthis.png",                            "Data/Atlases/System.xml",                        "achievement_029_canttouchthis")
	IMG("pak::bin\\graphics\\achievements\\030_maxhp.png",                                    "Data/Atlases/System.xml",                        "achievement_030_maxhp")
	IMG("pak::bin\\graphics\\achievements\\031_maxreload.png",                                "Data/Atlases/System.xml",                        "achievement_031_maxreload")
	IMG("pak::bin\\graphics\\achievements\\032_maxrateoffire.png",                            "Data/Atlases/System.xml",                        "achievement_032_maxrateoffire")
	IMG("pak::bin\\graphics\\achievements\\033_maxdamage.png",                                "Data/Atlases/System.xml",                        "achievement_033_maxdamage")
	IMG("pak::bin\\graphics\\achievements\\034_maxspeed.png",                                 "Data/Atlases/System.xml",                        "achievement_034_maxspeed")
	IMG("pak::bin\\graphics\\achievements\\035_maxammo.png",                                  "Data/Atlases/System.xml",                        "achievement_035_maxammo")
	IMG("pak::bin\\graphics\\achievements\\036_maxdiscount.png",                              "Data/Atlases/System.xml",                        "achievement_036_maxdiscount")
	IMG("pak::bin\\graphics\\achievements\\037_maxexperience.png",                            "Data/Atlases/System.xml",                        "achievement_037_maxexperience")
	IMG("pak::bin\\graphics\\achievements\\038_dungeoncrawler.png",                           "Data/Atlases/System.xml",                        "achievement_038_dungeoncrawler")
	IMG("pak::bin\\graphics\\achievements\\039_dungeonmaster.png",                            "Data/Atlases/System.xml",                        "achievement_039_dungeonmaster")
	IMG("pak::bin\\graphics\\achievements\\040_dungeonboss.png",                              "Data/Atlases/System.xml",                        "achievement_040_dungeonboss")
	IMG("pak::bin\\graphics\\achievements\\041_theghostisclear.png",                          "Data/Atlases/System.xml",                        "achievement_041_theghostisclear")
	IMG("pak::bin\\graphics\\achievements\\042_developerdeath.png",                           "Data/Atlases/System.xml",                        "achievement_042_developerdeath")
	IMG("pak::bin\\graphics\\achievements\\043_puddipuddipuddi.png",                          "Data/Atlases/System.xml",                        "achievement_043_puddipuddipuddi")
	IMG("pak::bin\\graphics\\achievements\\044_healinglevel1.png",                            "Data/Atlases/System.xml",                        "achievement_044_healinglevel1")
	IMG("pak::bin\\graphics\\achievements\\045_healinglevel2.png",                            "Data/Atlases/System.xml",                        "achievement_045_healinglevel2")
	IMG("pak::bin\\graphics\\achievements\\046_healinglevel3.png",                            "Data/Atlases/System.xml",                        "achievement_046_healinglevel3")
	IMG("pak::bin\\graphics\\achievements\\047_healinglevel4.png",                            "Data/Atlases/System.xml",                        "achievement_047_healinglevel4")
	IMG("pak::bin\\graphics\\achievements\\blank.png",                                        "Data/Atlases/System.xml",                        "achievement_blank")
	IMG("pak::bin\\graphics\\achievements\\locked.png",                                       "Data/Atlases/System.xml",                        "achievement_locked")
	IMG("pak::bin\\graphics\\achievements\\locked_clear.png",                                 "Data/Atlases/System.xml",                        "achievement_locked_clear")
	IMG("pak::bin\\graphics\\achievements\\unlocked.png",                                     "Data/Atlases/System.xml",                        "achievement_unlocked")
	IMG("pak::bin\\graphics\\achievements\\unlocked_clear.png",                               "Data/Atlases/System.xml",                        "achievement_unlocked_clear")
	IMG("pak::bin\\graphics\\geoip\\flags\\ad.png",                                           "Data/Atlases/System.xml",                        "flag_ad")
	IMG("pak::bin\\graphics\\geoip\\flags\\ae.png",                                           "Data/Atlases/System.xml",                        "flag_ae")
	IMG("pak::bin\\graphics\\geoip\\flags\\af.png",                                           "Data/Atlases/System.xml",                        "flag_af")
	IMG("pak::bin\\graphics\\geoip\\flags\\ag.png",                                           "Data/Atlases/System.xml",                        "flag_ag")
	IMG("pak::bin\\graphics\\geoip\\flags\\ai.png",                                           "Data/Atlases/System.xml",                        "flag_ai")
	IMG("pak::bin\\graphics\\geoip\\flags\\al.png",                                           "Data/Atlases/System.xml",                        "flag_al")
	IMG("pak::bin\\graphics\\geoip\\flags\\am.png",                                           "Data/Atlases/System.xml",                        "flag_am")
	IMG("pak::bin\\graphics\\geoip\\flags\\an.png",                                           "Data/Atlases/System.xml",                        "flag_an")
	IMG("pak::bin\\graphics\\geoip\\flags\\ao.png",                                           "Data/Atlases/System.xml",                        "flag_ao")
	IMG("pak::bin\\graphics\\geoip\\flags\\ar.png",                                           "Data/Atlases/System.xml",                        "flag_ar")
	IMG("pak::bin\\graphics\\geoip\\flags\\as.png",                                           "Data/Atlases/System.xml",                        "flag_as")
	IMG("pak::bin\\graphics\\geoip\\flags\\at.png",                                           "Data/Atlases/System.xml",                        "flag_at")
	IMG("pak::bin\\graphics\\geoip\\flags\\au.png",                                           "Data/Atlases/System.xml",                        "flag_au")
	IMG("pak::bin\\graphics\\geoip\\flags\\aw.png",                                           "Data/Atlases/System.xml",                        "flag_aw")
	IMG("pak::bin\\graphics\\geoip\\flags\\ax.png",                                           "Data/Atlases/System.xml",                        "flag_ax")
	IMG("pak::bin\\graphics\\geoip\\flags\\az.png",                                           "Data/Atlases/System.xml",                        "flag_az")
	IMG("pak::bin\\graphics\\geoip\\flags\\ba.png",                                           "Data/Atlases/System.xml",                        "flag_ba")
	IMG("pak::bin\\graphics\\geoip\\flags\\bb.png",                                           "Data/Atlases/System.xml",                        "flag_bb")
	IMG("pak::bin\\graphics\\geoip\\flags\\bd.png",                                           "Data/Atlases/System.xml",                        "flag_bd")
	IMG("pak::bin\\graphics\\geoip\\flags\\be.png",                                           "Data/Atlases/System.xml",                        "flag_be")
	IMG("pak::bin\\graphics\\geoip\\flags\\bf.png",                                           "Data/Atlases/System.xml",                        "flag_bf")
	IMG("pak::bin\\graphics\\geoip\\flags\\bg.png",                                           "Data/Atlases/System.xml",                        "flag_bg")
	IMG("pak::bin\\graphics\\geoip\\flags\\bh.png",                                           "Data/Atlases/System.xml",                        "flag_bh")
	IMG("pak::bin\\graphics\\geoip\\flags\\bi.png",                                           "Data/Atlases/System.xml",                        "flag_bi")
	IMG("pak::bin\\graphics\\geoip\\flags\\bj.png",                                           "Data/Atlases/System.xml",                        "flag_bj")
	IMG("pak::bin\\graphics\\geoip\\flags\\bm.png",                                           "Data/Atlases/System.xml",                        "flag_bm")
	IMG("pak::bin\\graphics\\geoip\\flags\\bn.png",                                           "Data/Atlases/System.xml",                        "flag_bn")
	IMG("pak::bin\\graphics\\geoip\\flags\\bo.png",                                           "Data/Atlases/System.xml",                        "flag_bo")
	IMG("pak::bin\\graphics\\geoip\\flags\\br.png",                                           "Data/Atlases/System.xml",                        "flag_br")
	IMG("pak::bin\\graphics\\geoip\\flags\\bs.png",                                           "Data/Atlases/System.xml",                        "flag_bs")
	IMG("pak::bin\\graphics\\geoip\\flags\\bt.png",                                           "Data/Atlases/System.xml",                        "flag_bt")
	IMG("pak::bin\\graphics\\geoip\\flags\\bv.png",                                           "Data/Atlases/System.xml",                        "flag_bv")
	IMG("pak::bin\\graphics\\geoip\\flags\\bw.png",                                           "Data/Atlases/System.xml",                        "flag_bw")
	IMG("pak::bin\\graphics\\geoip\\flags\\by.png",                                           "Data/Atlases/System.xml",                        "flag_by")
	IMG("pak::bin\\graphics\\geoip\\flags\\bz.png",                                           "Data/Atlases/System.xml",                        "flag_bz")
	IMG("pak::bin\\graphics\\geoip\\flags\\ca.png",                                           "Data/Atlases/System.xml",                        "flag_ca")
	IMG("pak::bin\\graphics\\geoip\\flags\\catalonia.png",                                    "Data/Atlases/System.xml",                        "flag_catalonia")
	IMG("pak::bin\\graphics\\geoip\\flags\\cc.png",                                           "Data/Atlases/System.xml",                        "flag_cc")
	IMG("pak::bin\\graphics\\geoip\\flags\\cd.png",                                           "Data/Atlases/System.xml",                        "flag_cd")
	IMG("pak::bin\\graphics\\geoip\\flags\\cf.png",                                           "Data/Atlases/System.xml",                        "flag_cf")
	IMG("pak::bin\\graphics\\geoip\\flags\\cg.png",                                           "Data/Atlases/System.xml",                        "flag_cg")
	IMG("pak::bin\\graphics\\geoip\\flags\\ch.png",                                           "Data/Atlases/System.xml",                        "flag_ch")
	IMG("pak::bin\\graphics\\geoip\\flags\\ci.png",                                           "Data/Atlases/System.xml",                        "flag_ci")
	IMG("pak::bin\\graphics\\geoip\\flags\\ck.png",                                           "Data/Atlases/System.xml",                        "flag_ck")
	IMG("pak::bin\\graphics\\geoip\\flags\\cl.png",                                           "Data/Atlases/System.xml",                        "flag_cl")
	IMG("pak::bin\\graphics\\geoip\\flags\\cm.png",                                           "Data/Atlases/System.xml",                        "flag_cm")
	IMG("pak::bin\\graphics\\geoip\\flags\\cn.png",                                           "Data/Atlases/System.xml",                        "flag_cn")
	IMG("pak::bin\\graphics\\geoip\\flags\\co.png",                                           "Data/Atlases/System.xml",                        "flag_co")
	IMG("pak::bin\\graphics\\geoip\\flags\\cr.png",                                           "Data/Atlases/System.xml",                        "flag_cr")
	IMG("pak::bin\\graphics\\geoip\\flags\\cs.png",                                           "Data/Atlases/System.xml",                        "flag_cs")
	IMG("pak::bin\\graphics\\geoip\\flags\\cu.png",                                           "Data/Atlases/System.xml",                        "flag_cu")
	IMG("pak::bin\\graphics\\geoip\\flags\\cv.png",                                           "Data/Atlases/System.xml",                        "flag_cv")
	IMG("pak::bin\\graphics\\geoip\\flags\\cx.png",                                           "Data/Atlases/System.xml",                        "flag_cx")
	IMG("pak::bin\\graphics\\geoip\\flags\\cy.png",                                           "Data/Atlases/System.xml",                        "flag_cy")
	IMG("pak::bin\\graphics\\geoip\\flags\\cz.png",                                           "Data/Atlases/System.xml",                        "flag_cz")
	IMG("pak::bin\\graphics\\geoip\\flags\\de.png",                                           "Data/Atlases/System.xml",                        "flag_de")
	IMG("pak::bin\\graphics\\geoip\\flags\\dj.png",                                           "Data/Atlases/System.xml",                        "flag_dj")
	IMG("pak::bin\\graphics\\geoip\\flags\\dk.png",                                           "Data/Atlases/System.xml",                        "flag_dk")
	IMG("pak::bin\\graphics\\geoip\\flags\\dm.png",                                           "Data/Atlases/System.xml",                        "flag_dm")
	IMG("pak::bin\\graphics\\geoip\\flags\\do.png",                                           "Data/Atlases/System.xml",                        "flag_do")
	IMG("pak::bin\\graphics\\geoip\\flags\\dz.png",                                           "Data/Atlases/System.xml",                        "flag_dz")
	IMG("pak::bin\\graphics\\geoip\\flags\\ec.png",                                           "Data/Atlases/System.xml",                        "flag_ec")
	IMG("pak::bin\\graphics\\geoip\\flags\\ee.png",                                           "Data/Atlases/System.xml",                        "flag_ee")
	IMG("pak::bin\\graphics\\geoip\\flags\\eg.png",                                           "Data/Atlases/System.xml",                        "flag_eg")
	IMG("pak::bin\\graphics\\geoip\\flags\\eh.png",                                           "Data/Atlases/System.xml",                        "flag_eh")
	IMG("pak::bin\\graphics\\geoip\\flags\\england.png",                                      "Data/Atlases/System.xml",                        "flag_england")
	IMG("pak::bin\\graphics\\geoip\\flags\\er.png",                                           "Data/Atlases/System.xml",                        "flag_er")
	IMG("pak::bin\\graphics\\geoip\\flags\\es.png",                                           "Data/Atlases/System.xml",                        "flag_es")
	IMG("pak::bin\\graphics\\geoip\\flags\\et.png",                                           "Data/Atlases/System.xml",                        "flag_et")
	IMG("pak::bin\\graphics\\geoip\\flags\\europeanunion.png",                                "Data/Atlases/System.xml",                        "flag_europeanunion")
	IMG("pak::bin\\graphics\\geoip\\flags\\fam.png",                                          "Data/Atlases/System.xml",                        "flag_fam")
	IMG("pak::bin\\graphics\\geoip\\flags\\fi.png",                                           "Data/Atlases/System.xml",                        "flag_fi")
	IMG("pak::bin\\graphics\\geoip\\flags\\fj.png",                                           "Data/Atlases/System.xml",                        "flag_fj")
	IMG("pak::bin\\graphics\\geoip\\flags\\fk.png",                                           "Data/Atlases/System.xml",                        "flag_fk")
	IMG("pak::bin\\graphics\\geoip\\flags\\fm.png",                                           "Data/Atlases/System.xml",                        "flag_fm")
	IMG("pak::bin\\graphics\\geoip\\flags\\fo.png",                                           "Data/Atlases/System.xml",                        "flag_fo")
	IMG("pak::bin\\graphics\\geoip\\flags\\fr.png",                                           "Data/Atlases/System.xml",                        "flag_fr")
	IMG("pak::bin\\graphics\\geoip\\flags\\ga.png",                                           "Data/Atlases/System.xml",                        "flag_ga")
	IMG("pak::bin\\graphics\\geoip\\flags\\gb.png",                                           "Data/Atlases/System.xml",                        "flag_gb")
	IMG("pak::bin\\graphics\\geoip\\flags\\gd.png",                                           "Data/Atlases/System.xml",                        "flag_gd")
	IMG("pak::bin\\graphics\\geoip\\flags\\ge.png",                                           "Data/Atlases/System.xml",                        "flag_ge")
	IMG("pak::bin\\graphics\\geoip\\flags\\gf.png",                                           "Data/Atlases/System.xml",                        "flag_gf")
	IMG("pak::bin\\graphics\\geoip\\flags\\gh.png",                                           "Data/Atlases/System.xml",                        "flag_gh")
	IMG("pak::bin\\graphics\\geoip\\flags\\gi.png",                                           "Data/Atlases/System.xml",                        "flag_gi")
	IMG("pak::bin\\graphics\\geoip\\flags\\gl.png",                                           "Data/Atlases/System.xml",                        "flag_gl")
	IMG("pak::bin\\graphics\\geoip\\flags\\gm.png",                                           "Data/Atlases/System.xml",                        "flag_gm")
	IMG("pak::bin\\graphics\\geoip\\flags\\gn.png",                                           "Data/Atlases/System.xml",                        "flag_gn")
	IMG("pak::bin\\graphics\\geoip\\flags\\gp.png",                                           "Data/Atlases/System.xml",                        "flag_gp")
	IMG("pak::bin\\graphics\\geoip\\flags\\gq.png",                                           "Data/Atlases/System.xml",                        "flag_gq")
	IMG("pak::bin\\graphics\\geoip\\flags\\gr.png",                                           "Data/Atlases/System.xml",                        "flag_gr")
	IMG("pak::bin\\graphics\\geoip\\flags\\gs.png",                                           "Data/Atlases/System.xml",                        "flag_gs")
	IMG("pak::bin\\graphics\\geoip\\flags\\gt.png",                                           "Data/Atlases/System.xml",                        "flag_gt")
	IMG("pak::bin\\graphics\\geoip\\flags\\gu.png",                                           "Data/Atlases/System.xml",                        "flag_gu")
	IMG("pak::bin\\graphics\\geoip\\flags\\gw.png",                                           "Data/Atlases/System.xml",                        "flag_gw")
	IMG("pak::bin\\graphics\\geoip\\flags\\gy.png",                                           "Data/Atlases/System.xml",                        "flag_gy")
	IMG("pak::bin\\graphics\\geoip\\flags\\hk.png",                                           "Data/Atlases/System.xml",                        "flag_hk")
	IMG("pak::bin\\graphics\\geoip\\flags\\hm.png",                                           "Data/Atlases/System.xml",                        "flag_hm")
	IMG("pak::bin\\graphics\\geoip\\flags\\hn.png",                                           "Data/Atlases/System.xml",                        "flag_hn")
	IMG("pak::bin\\graphics\\geoip\\flags\\hr.png",                                           "Data/Atlases/System.xml",                        "flag_hr")
	IMG("pak::bin\\graphics\\geoip\\flags\\ht.png",                                           "Data/Atlases/System.xml",                        "flag_ht")
	IMG("pak::bin\\graphics\\geoip\\flags\\hu.png",                                           "Data/Atlases/System.xml",                        "flag_hu")
	IMG("pak::bin\\graphics\\geoip\\flags\\id.png",                                           "Data/Atlases/System.xml",                        "flag_id")
	IMG("pak::bin\\graphics\\geoip\\flags\\ie.png",                                           "Data/Atlases/System.xml",                        "flag_ie")
	IMG("pak::bin\\graphics\\geoip\\flags\\il.png",                                           "Data/Atlases/System.xml",                        "flag_il")
	IMG("pak::bin\\graphics\\geoip\\flags\\in.png",                                           "Data/Atlases/System.xml",                        "flag_in")
	IMG("pak::bin\\graphics\\geoip\\flags\\io.png",                                           "Data/Atlases/System.xml",                        "flag_io")
	IMG("pak::bin\\graphics\\geoip\\flags\\iq.png",                                           "Data/Atlases/System.xml",                        "flag_iq")
	IMG("pak::bin\\graphics\\geoip\\flags\\ir.png",                                           "Data/Atlases/System.xml",                        "flag_ir")
	IMG("pak::bin\\graphics\\geoip\\flags\\is.png",                                           "Data/Atlases/System.xml",                        "flag_is")
	IMG("pak::bin\\graphics\\geoip\\flags\\it.png",                                           "Data/Atlases/System.xml",                        "flag_it")
	IMG("pak::bin\\graphics\\geoip\\flags\\jm.png",                                           "Data/Atlases/System.xml",                        "flag_jm")
	IMG("pak::bin\\graphics\\geoip\\flags\\jo.png",                                           "Data/Atlases/System.xml",                        "flag_jo")
	IMG("pak::bin\\graphics\\geoip\\flags\\jp.png",                                           "Data/Atlases/System.xml",                        "flag_jp")
	IMG("pak::bin\\graphics\\geoip\\flags\\ke.png",                                           "Data/Atlases/System.xml",                        "flag_ke")
	IMG("pak::bin\\graphics\\geoip\\flags\\kg.png",                                           "Data/Atlases/System.xml",                        "flag_kg")
	IMG("pak::bin\\graphics\\geoip\\flags\\kh.png",                                           "Data/Atlases/System.xml",                        "flag_kh")
	IMG("pak::bin\\graphics\\geoip\\flags\\ki.png",                                           "Data/Atlases/System.xml",                        "flag_ki")
	IMG("pak::bin\\graphics\\geoip\\flags\\km.png",                                           "Data/Atlases/System.xml",                        "flag_km")
	IMG("pak::bin\\graphics\\geoip\\flags\\kn.png",                                           "Data/Atlases/System.xml",                        "flag_kn")
	IMG("pak::bin\\graphics\\geoip\\flags\\kp.png",                                           "Data/Atlases/System.xml",                        "flag_kp")
	IMG("pak::bin\\graphics\\geoip\\flags\\kr.png",                                           "Data/Atlases/System.xml",                        "flag_kr")
	IMG("pak::bin\\graphics\\geoip\\flags\\kw.png",                                           "Data/Atlases/System.xml",                        "flag_kw")
	IMG("pak::bin\\graphics\\geoip\\flags\\ky.png",                                           "Data/Atlases/System.xml",                        "flag_ky")
	IMG("pak::bin\\graphics\\geoip\\flags\\kz.png",                                           "Data/Atlases/System.xml",                        "flag_kz")
	IMG("pak::bin\\graphics\\geoip\\flags\\la.png",                                           "Data/Atlases/System.xml",                        "flag_la")
	IMG("pak::bin\\graphics\\geoip\\flags\\lb.png",                                           "Data/Atlases/System.xml",                        "flag_lb")
	IMG("pak::bin\\graphics\\geoip\\flags\\lc.png",                                           "Data/Atlases/System.xml",                        "flag_lc")
	IMG("pak::bin\\graphics\\geoip\\flags\\li.png",                                           "Data/Atlases/System.xml",                        "flag_li")
	IMG("pak::bin\\graphics\\geoip\\flags\\lk.png",                                           "Data/Atlases/System.xml",                        "flag_lk")
	IMG("pak::bin\\graphics\\geoip\\flags\\lr.png",                                           "Data/Atlases/System.xml",                        "flag_lr")
	IMG("pak::bin\\graphics\\geoip\\flags\\ls.png",                                           "Data/Atlases/System.xml",                        "flag_ls")
	IMG("pak::bin\\graphics\\geoip\\flags\\lt.png",                                           "Data/Atlases/System.xml",                        "flag_lt")
	IMG("pak::bin\\graphics\\geoip\\flags\\lu.png",                                           "Data/Atlases/System.xml",                        "flag_lu")
	IMG("pak::bin\\graphics\\geoip\\flags\\lv.png",                                           "Data/Atlases/System.xml",                        "flag_lv")
	IMG("pak::bin\\graphics\\geoip\\flags\\ly.png",                                           "Data/Atlases/System.xml",                        "flag_ly")
	IMG("pak::bin\\graphics\\geoip\\flags\\ma.png",                                           "Data/Atlases/System.xml",                        "flag_ma")
	IMG("pak::bin\\graphics\\geoip\\flags\\mc.png",                                           "Data/Atlases/System.xml",                        "flag_mc")
	IMG("pak::bin\\graphics\\geoip\\flags\\md.png",                                           "Data/Atlases/System.xml",                        "flag_md")
	IMG("pak::bin\\graphics\\geoip\\flags\\me.png",                                           "Data/Atlases/System.xml",                        "flag_me")
	IMG("pak::bin\\graphics\\geoip\\flags\\mg.png",                                           "Data/Atlases/System.xml",                        "flag_mg")
	IMG("pak::bin\\graphics\\geoip\\flags\\mh.png",                                           "Data/Atlases/System.xml",                        "flag_mh")
	IMG("pak::bin\\graphics\\geoip\\flags\\mk.png",                                           "Data/Atlases/System.xml",                        "flag_mk")
	IMG("pak::bin\\graphics\\geoip\\flags\\ml.png",                                           "Data/Atlases/System.xml",                        "flag_ml")
	IMG("pak::bin\\graphics\\geoip\\flags\\mm.png",                                           "Data/Atlases/System.xml",                        "flag_mm")
	IMG("pak::bin\\graphics\\geoip\\flags\\mn.png",                                           "Data/Atlases/System.xml",                        "flag_mn")
	IMG("pak::bin\\graphics\\geoip\\flags\\mo.png",                                           "Data/Atlases/System.xml",                        "flag_mo")
	IMG("pak::bin\\graphics\\geoip\\flags\\mp.png",                                           "Data/Atlases/System.xml",                        "flag_mp")
	IMG("pak::bin\\graphics\\geoip\\flags\\mq.png",                                           "Data/Atlases/System.xml",                        "flag_mq")
	IMG("pak::bin\\graphics\\geoip\\flags\\mr.png",                                           "Data/Atlases/System.xml",                        "flag_mr")
	IMG("pak::bin\\graphics\\geoip\\flags\\ms.png",                                           "Data/Atlases/System.xml",                        "flag_ms")
	IMG("pak::bin\\graphics\\geoip\\flags\\mt.png",                                           "Data/Atlases/System.xml",                        "flag_mt")
	IMG("pak::bin\\graphics\\geoip\\flags\\mu.png",                                           "Data/Atlases/System.xml",                        "flag_mu")
	IMG("pak::bin\\graphics\\geoip\\flags\\mv.png",                                           "Data/Atlases/System.xml",                        "flag_mv")
	IMG("pak::bin\\graphics\\geoip\\flags\\mw.png",                                           "Data/Atlases/System.xml",                        "flag_mw")
	IMG("pak::bin\\graphics\\geoip\\flags\\mx.png",                                           "Data/Atlases/System.xml",                        "flag_mx")
	IMG("pak::bin\\graphics\\geoip\\flags\\my.png",                                           "Data/Atlases/System.xml",                        "flag_my")
	IMG("pak::bin\\graphics\\geoip\\flags\\mz.png",                                           "Data/Atlases/System.xml",                        "flag_mz")
	IMG("pak::bin\\graphics\\geoip\\flags\\na.png",                                           "Data/Atlases/System.xml",                        "flag_na")
	IMG("pak::bin\\graphics\\geoip\\flags\\nc.png",                                           "Data/Atlases/System.xml",                        "flag_nc")
	IMG("pak::bin\\graphics\\geoip\\flags\\ne.png",                                           "Data/Atlases/System.xml",                        "flag_ne")
	IMG("pak::bin\\graphics\\geoip\\flags\\nf.png",                                           "Data/Atlases/System.xml",                        "flag_nf")
	IMG("pak::bin\\graphics\\geoip\\flags\\ng.png",                                           "Data/Atlases/System.xml",                        "flag_ng")
	IMG("pak::bin\\graphics\\geoip\\flags\\ni.png",                                           "Data/Atlases/System.xml",                        "flag_ni")
	IMG("pak::bin\\graphics\\geoip\\flags\\nl.png",                                           "Data/Atlases/System.xml",                        "flag_nl")
	IMG("pak::bin\\graphics\\geoip\\flags\\no.png",                                           "Data/Atlases/System.xml",                        "flag_no")
	IMG("pak::bin\\graphics\\geoip\\flags\\np.png",                                           "Data/Atlases/System.xml",                        "flag_np")
	IMG("pak::bin\\graphics\\geoip\\flags\\nr.png",                                           "Data/Atlases/System.xml",                        "flag_nr")
	IMG("pak::bin\\graphics\\geoip\\flags\\nu.png",                                           "Data/Atlases/System.xml",                        "flag_nu")
	IMG("pak::bin\\graphics\\geoip\\flags\\nz.png",                                           "Data/Atlases/System.xml",                        "flag_nz")
	IMG("pak::bin\\graphics\\geoip\\flags\\om.png",                                           "Data/Atlases/System.xml",                        "flag_om")
	IMG("pak::bin\\graphics\\geoip\\flags\\pa.png",                                           "Data/Atlases/System.xml",                        "flag_pa")
	IMG("pak::bin\\graphics\\geoip\\flags\\pe.png",                                           "Data/Atlases/System.xml",                        "flag_pe")
	IMG("pak::bin\\graphics\\geoip\\flags\\pf.png",                                           "Data/Atlases/System.xml",                        "flag_pf")
	IMG("pak::bin\\graphics\\geoip\\flags\\pg.png",                                           "Data/Atlases/System.xml",                        "flag_pg")
	IMG("pak::bin\\graphics\\geoip\\flags\\ph.png",                                           "Data/Atlases/System.xml",                        "flag_ph")
	IMG("pak::bin\\graphics\\geoip\\flags\\pk.png",                                           "Data/Atlases/System.xml",                        "flag_pk")
	IMG("pak::bin\\graphics\\geoip\\flags\\pl.png",                                           "Data/Atlases/System.xml",                        "flag_pl")
	IMG("pak::bin\\graphics\\geoip\\flags\\pm.png",                                           "Data/Atlases/System.xml",                        "flag_pm")
	IMG("pak::bin\\graphics\\geoip\\flags\\pn.png",                                           "Data/Atlases/System.xml",                        "flag_pn")
	IMG("pak::bin\\graphics\\geoip\\flags\\pr.png",                                           "Data/Atlases/System.xml",                        "flag_pr")
	IMG("pak::bin\\graphics\\geoip\\flags\\ps.png",                                           "Data/Atlases/System.xml",                        "flag_ps")
	IMG("pak::bin\\graphics\\geoip\\flags\\pt.png",                                           "Data/Atlases/System.xml",                        "flag_pt")
	IMG("pak::bin\\graphics\\geoip\\flags\\pw.png",                                           "Data/Atlases/System.xml",                        "flag_pw")
	IMG("pak::bin\\graphics\\geoip\\flags\\py.png",                                           "Data/Atlases/System.xml",                        "flag_py")
	IMG("pak::bin\\graphics\\geoip\\flags\\qa.png",                                           "Data/Atlases/System.xml",                        "flag_qa")
	IMG("pak::bin\\graphics\\geoip\\flags\\re.png",                                           "Data/Atlases/System.xml",                        "flag_re")
	IMG("pak::bin\\graphics\\geoip\\flags\\ro.png",                                           "Data/Atlases/System.xml",                        "flag_ro")
	IMG("pak::bin\\graphics\\geoip\\flags\\rs.png",                                           "Data/Atlases/System.xml",                        "flag_rs")
	IMG("pak::bin\\graphics\\geoip\\flags\\ru.png",                                           "Data/Atlases/System.xml",                        "flag_ru")
	IMG("pak::bin\\graphics\\geoip\\flags\\rw.png",                                           "Data/Atlases/System.xml",                        "flag_rw")
	IMG("pak::bin\\graphics\\geoip\\flags\\sa.png",                                           "Data/Atlases/System.xml",                        "flag_sa")
	IMG("pak::bin\\graphics\\geoip\\flags\\sb.png",                                           "Data/Atlases/System.xml",                        "flag_sb")
	IMG("pak::bin\\graphics\\geoip\\flags\\sc.png",                                           "Data/Atlases/System.xml",                        "flag_sc")
	IMG("pak::bin\\graphics\\geoip\\flags\\scotland.png",                                     "Data/Atlases/System.xml",                        "flag_scotland")
	IMG("pak::bin\\graphics\\geoip\\flags\\sd.png",                                           "Data/Atlases/System.xml",                        "flag_sd")
	IMG("pak::bin\\graphics\\geoip\\flags\\se.png",                                           "Data/Atlases/System.xml",                        "flag_se")
	IMG("pak::bin\\graphics\\geoip\\flags\\sg.png",                                           "Data/Atlases/System.xml",                        "flag_sg")
	IMG("pak::bin\\graphics\\geoip\\flags\\sh.png",                                           "Data/Atlases/System.xml",                        "flag_sh")
	IMG("pak::bin\\graphics\\geoip\\flags\\si.png",                                           "Data/Atlases/System.xml",                        "flag_si")
	IMG("pak::bin\\graphics\\geoip\\flags\\sj.png",                                           "Data/Atlases/System.xml",                        "flag_sj")
	IMG("pak::bin\\graphics\\geoip\\flags\\sk.png",                                           "Data/Atlases/System.xml",                        "flag_sk")
	IMG("pak::bin\\graphics\\geoip\\flags\\sl.png",                                           "Data/Atlases/System.xml",                        "flag_sl")
	IMG("pak::bin\\graphics\\geoip\\flags\\sm.png",                                           "Data/Atlases/System.xml",                        "flag_sm")
	IMG("pak::bin\\graphics\\geoip\\flags\\sn.png",                                           "Data/Atlases/System.xml",                        "flag_sn")
	IMG("pak::bin\\graphics\\geoip\\flags\\so.png",                                           "Data/Atlases/System.xml",                        "flag_so")
	IMG("pak::bin\\graphics\\geoip\\flags\\sr.png",                                           "Data/Atlases/System.xml",                        "flag_sr")
	IMG("pak::bin\\graphics\\geoip\\flags\\st.png",                                           "Data/Atlases/System.xml",                        "flag_st")
	IMG("pak::bin\\graphics\\geoip\\flags\\sv.png",                                           "Data/Atlases/System.xml",                        "flag_sv")
	IMG("pak::bin\\graphics\\geoip\\flags\\sy.png",                                           "Data/Atlases/System.xml",                        "flag_sy")
	IMG("pak::bin\\graphics\\geoip\\flags\\sz.png",                                           "Data/Atlases/System.xml",                        "flag_sz")
	IMG("pak::bin\\graphics\\geoip\\flags\\tc.png",                                           "Data/Atlases/System.xml",                        "flag_tc")
	IMG("pak::bin\\graphics\\geoip\\flags\\td.png",                                           "Data/Atlases/System.xml",                        "flag_td")
	IMG("pak::bin\\graphics\\geoip\\flags\\tf.png",                                           "Data/Atlases/System.xml",                        "flag_tf")
	IMG("pak::bin\\graphics\\geoip\\flags\\tg.png",                                           "Data/Atlases/System.xml",                        "flag_tg")
	IMG("pak::bin\\graphics\\geoip\\flags\\th.png",                                           "Data/Atlases/System.xml",                        "flag_th")
	IMG("pak::bin\\graphics\\geoip\\flags\\tj.png",                                           "Data/Atlases/System.xml",                        "flag_tj")
	IMG("pak::bin\\graphics\\geoip\\flags\\tk.png",                                           "Data/Atlases/System.xml",                        "flag_tk")
	IMG("pak::bin\\graphics\\geoip\\flags\\tl.png",                                           "Data/Atlases/System.xml",                        "flag_tl")
	IMG("pak::bin\\graphics\\geoip\\flags\\tm.png",                                           "Data/Atlases/System.xml",                        "flag_tm")
	IMG("pak::bin\\graphics\\geoip\\flags\\tn.png",                                           "Data/Atlases/System.xml",                        "flag_tn")
	IMG("pak::bin\\graphics\\geoip\\flags\\to.png",                                           "Data/Atlases/System.xml",                        "flag_to")
	IMG("pak::bin\\graphics\\geoip\\flags\\tr.png",                                           "Data/Atlases/System.xml",                        "flag_tr")
	IMG("pak::bin\\graphics\\geoip\\flags\\tt.png",                                           "Data/Atlases/System.xml",                        "flag_tt")
	IMG("pak::bin\\graphics\\geoip\\flags\\tv.png",                                           "Data/Atlases/System.xml",                        "flag_tv")
	IMG("pak::bin\\graphics\\geoip\\flags\\tw.png",                                           "Data/Atlases/System.xml",                        "flag_tw")
	IMG("pak::bin\\graphics\\geoip\\flags\\tz.png",                                           "Data/Atlases/System.xml",                        "flag_tz")
	IMG("pak::bin\\graphics\\geoip\\flags\\ua.png",                                           "Data/Atlases/System.xml",                        "flag_ua")
	IMG("pak::bin\\graphics\\geoip\\flags\\ug.png",                                           "Data/Atlases/System.xml",                        "flag_ug")
	IMG("pak::bin\\graphics\\geoip\\flags\\um.png",                                           "Data/Atlases/System.xml",                        "flag_um")
	IMG("pak::bin\\graphics\\geoip\\flags\\us.png",                                           "Data/Atlases/System.xml",                        "flag_us")
	IMG("pak::bin\\graphics\\geoip\\flags\\uy.png",                                           "Data/Atlases/System.xml",                        "flag_uy")
	IMG("pak::bin\\graphics\\geoip\\flags\\uz.png",                                           "Data/Atlases/System.xml",                        "flag_uz")
	IMG("pak::bin\\graphics\\geoip\\flags\\va.png",                                           "Data/Atlases/System.xml",                        "flag_va")
	IMG("pak::bin\\graphics\\geoip\\flags\\vc.png",                                           "Data/Atlases/System.xml",                        "flag_vc")
	IMG("pak::bin\\graphics\\geoip\\flags\\ve.png",                                           "Data/Atlases/System.xml",                        "flag_ve")
	IMG("pak::bin\\graphics\\geoip\\flags\\vg.png",                                           "Data/Atlases/System.xml",                        "flag_vg")
	IMG("pak::bin\\graphics\\geoip\\flags\\vi.png",                                           "Data/Atlases/System.xml",                        "flag_vi")
	IMG("pak::bin\\graphics\\geoip\\flags\\vn.png",                                           "Data/Atlases/System.xml",                        "flag_vn")
	IMG("pak::bin\\graphics\\geoip\\flags\\vu.png",                                           "Data/Atlases/System.xml",                        "flag_vu")
	IMG("pak::bin\\graphics\\geoip\\flags\\wales.png",                                        "Data/Atlases/System.xml",                        "flag_wales")
	IMG("pak::bin\\graphics\\geoip\\flags\\wf.png",                                           "Data/Atlases/System.xml",                        "flag_wf")
	IMG("pak::bin\\graphics\\geoip\\flags\\ws.png",                                           "Data/Atlases/System.xml",                        "flag_ws")
	IMG("pak::bin\\graphics\\geoip\\flags\\ye.png",                                           "Data/Atlases/System.xml",                        "flag_ye")
	IMG("pak::bin\\graphics\\geoip\\flags\\yt.png",                                           "Data/Atlases/System.xml",                        "flag_yt")
	IMG("pak::bin\\graphics\\geoip\\flags\\za.png",                                           "Data/Atlases/System.xml",                        "flag_za")
	IMG("pak::bin\\graphics\\geoip\\flags\\zm.png",                                           "Data/Atlases/System.xml",                        "flag_zm")
	IMG("pak::bin\\graphics\\geoip\\flags\\zw.png",                                           "Data/Atlases/System.xml",                        "flag_zw")
	IMG("pak::bin\\graphics\\hud\\apple.png",                                                 "Data/Atlases/HUD.xml",                           "game_hud_apple")
	IMG("pak::bin\\graphics\\hud\\dead_text.png",                                             "Data/Atlases/HUD.xml",                           "game_hud_dead_text")
	IMG("pak::bin\\graphics\\hud\\health_bar.png",                                            "Data/Atlases/HUD.xml",                           "game_hud_health_bar")
	IMG("pak::bin\\graphics\\hud\\health_bar_border.png",                                     "Data/Atlases/HUD.xml",                           "game_hud_health_bar_border")
	IMG("pak::bin\\graphics\\hud\\hud_icons.png",                                             "Data/Atlases/HUD.xml",                           "game_hud_hud_icons")
	IMG("pak::bin\\graphics\\hud\\infinite_symbol.png",                                       "Data/Atlases/HUD.xml",                           "game_hud_infinite_symbol")
	IMG("pak::bin\\graphics\\hud\\invisible.png",                                             "Data/Atlases/HUD.xml",                           "game_hud_invisible")
	IMG("pak::bin\\graphics\\hud\\linux.png",                                                 "Data/Atlases/HUD.xml",                           "game_hud_linux")
	IMG("pak::bin\\graphics\\hud\\lock.png",                                                  "Data/Atlases/HUD.xml",                           "game_hud_lock")
	IMG("pak::bin\\graphics\\hud\\minimap.png",                                               "Data/Atlases/HUD.xml",                           "game_hud_minimap")
	IMG("pak::bin\\graphics\\hud\\player_compass_pip.png",                                    "Data/Atlases/HUD.xml",                           "game_hud_player_compass_pip")
	IMG("pak::bin\\graphics\\hud\\player_icons.png",                                          "Data/Atlases/HUD.xml",                           "game_hud_player_icons")
	IMG("pak::bin\\graphics\\hud\\player_icons_large.png",                                    "Data/Atlases/HUD.xml",                           "game_hud_player_icons_large")
	IMG("pak::bin\\graphics\\hud\\score_bg.png",                                              "Data/Atlases/HUD.xml",                           "game_hud_score_bg")
	IMG("pak::bin\\graphics\\hud\\score_bg_full.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_score_bg_full")
	IMG("pak::bin\\graphics\\hud\\visible.png",                                               "Data/Atlases/HUD.xml",                           "game_hud_visible")
	IMG("pak::bin\\graphics\\hud\\win32.png",                                                 "Data/Atlases/HUD.xml",                           "game_hud_win32")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\acid.png",                                      "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_acid")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\ak47.png",                                      "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_ak47")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\ar.png",                                        "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_ar")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\bolt_gun.png",                                  "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_bolt_gun")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\buff_grenade_ammo.png",                         "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_buff_grenade_ammo")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\buff_grenade_damage.png",                       "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_buff_grenade_damage")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\buff_grenade_health.png",                       "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_buff_grenade_health")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\buff_grenade_price.png",                        "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_buff_grenade_price")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\buff_grenade_reload.png",                       "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_buff_grenade_reload")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\buff_grenade_rof.png",                          "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_buff_grenade_rof")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\buff_grenade_speed.png",                        "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_buff_grenade_speed")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\buff_grenade_xp.png",                           "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_buff_grenade_xp")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\confetti.png",                                  "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_confetti")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\explosive.png",                                 "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_explosive")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\flare.png",                                     "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_flare")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\freeze.png",                                    "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_freeze")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\grenade.png",                                   "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_grenade")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\incendiary.png",                                "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_incendiary")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\laser.png",                                     "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_laser")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\love.png",                                      "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_love")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\m16.png",                                       "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_m16")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\magnum.png",                                    "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_magnum")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\melee.png",                                     "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_melee")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\pistol.png",                                    "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_pistol")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\rocket.png",                                    "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_rocket")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\shock_rifle.png",                               "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_shock_rifle")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\shotgun.png",                                   "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_shotgun")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\smg.png",                                       "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_smg")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\sticky.png",                                    "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_sticky")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\tripmine.png",                                  "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_tripmine")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\turret.png",                                    "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_turret")
	IMG("pak::bin\\graphics\\hud\\ammo icons\\winchester_rifle.png",                          "Data/Atlases/HUD.xml",                           "game_hud_ammoicon_winchester_rifle")
	IMG("pak::bin\\graphics\\hud\\chat\\chat_border.png",                                     "Data/Atlases/HUD.xml",                           "game_hud_chat_chat_border")
	IMG("pak::bin\\graphics\\hud\\chat\\chat_gradient_end.png",                               "Data/Atlases/HUD.xml",                           "game_hud_chat_chat_gradient_end")
	IMG("pak::bin\\graphics\\hud\\chat\\scroll_bar.png",                                      "Data/Atlases/HUD.xml",                           "game_hud_chat_scroll_bar")
	IMG("pak::bin\\graphics\\hud\\emblems\\developer.png",                                    "Data/Atlases/HUD.xml",                           "game_hud_emblem_developer")
	IMG("pak::bin\\graphics\\hud\\emblems\\donator.png",                                      "Data/Atlases/HUD.xml",                           "game_hud_emblem_donator")
	IMG("pak::bin\\graphics\\hud\\emblems\\large_donator.png",                                "Data/Atlases/HUD.xml",                           "game_hud_emblem_large_donator")
	IMG("pak::bin\\graphics\\hud\\emblems\\moderator.png",                                    "Data/Atlases/HUD.xml",                           "game_hud_emblem_moderator")
	IMG("pak::bin\\graphics\\hud\\game modes\\capture the bag\\bags.png",                     "Data/Atlases/HUD.xml",                           "game_hud_bags")
	IMG("pak::bin\\graphics\\hud\\game modes\\deathmatch\\position.png",                      "Data/Atlases/HUD.xml",                           "game_hud_position")
	IMG("pak::bin\\graphics\\hud\\game modes\\rack em up\\position.png",                      "Data/Atlases/HUD.xml",                           "game_hud_position")
	IMG("pak::bin\\graphics\\hud\\game modes\\team deathmatch\\position.png",                 "Data/Atlases/HUD.xml",                           "game_hud_position")
	IMG("pak::bin\\graphics\\hud\\keys\\all_keys.png",                                        "Data/Atlases/HUD.xml",                           "game_hud_key_keys")
	IMG("pak::bin\\graphics\\hud\\keys\\keyboard_down.png",                                   "Data/Atlases/HUD.xml",                           "game_hud_key_down")
	IMG("pak::bin\\graphics\\hud\\keys\\keyboard_up.png",                                     "Data/Atlases/HUD.xml",                           "game_hud_key_up")
	IMG("pak::bin\\graphics\\hud\\keys\\key_230.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_230")
	IMG("pak::bin\\graphics\\hud\\keys\\key_231.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_231")
	IMG("pak::bin\\graphics\\hud\\keys\\key_232.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_232")
	IMG("pak::bin\\graphics\\hud\\keys\\key_233.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_233")
	IMG("pak::bin\\graphics\\hud\\keys\\key_234.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_234")
	IMG("pak::bin\\graphics\\hud\\keys\\key_235.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_235")
	IMG("pak::bin\\graphics\\hud\\keys\\key_236.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_236")
	IMG("pak::bin\\graphics\\hud\\keys\\key_237.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_237")
	IMG("pak::bin\\graphics\\hud\\keys\\key_240.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_240")
	IMG("pak::bin\\graphics\\hud\\keys\\key_241.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_241")
	IMG("pak::bin\\graphics\\hud\\keys\\key_242.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_242")
	IMG("pak::bin\\graphics\\hud\\keys\\key_243.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_243")
	IMG("pak::bin\\graphics\\hud\\keys\\key_244.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_244")
	IMG("pak::bin\\graphics\\hud\\keys\\key_245.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_245")
	IMG("pak::bin\\graphics\\hud\\keys\\key_246.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_246")
	IMG("pak::bin\\graphics\\hud\\keys\\key_247.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_247")
	IMG("pak::bin\\graphics\\hud\\keys\\key_248.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_248")
	IMG("pak::bin\\graphics\\hud\\keys\\key_249.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_249")
	IMG("pak::bin\\graphics\\hud\\keys\\key_250.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_250")
	IMG("pak::bin\\graphics\\hud\\keys\\key_251.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_251")
	IMG("pak::bin\\graphics\\hud\\keys\\key_252.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_252")
	IMG("pak::bin\\graphics\\hud\\keys\\key_253.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_253")
	IMG("pak::bin\\graphics\\hud\\keys\\key_254.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_254")
	IMG("pak::bin\\graphics\\hud\\keys\\key_255.png",                                         "Data/Atlases/HUD.xml",                           "game_hud_key_255")
	IMG("pak::bin\\graphics\\hud\\message box\\background.png",                               "Data/Atlases/HUD.xml",                           "game_hud_messagebox_background")
	IMG("pak::bin\\graphics\\hud\\message box\\close_pip.png",                                "Data/Atlases/HUD.xml",                           "game_hud_messagebox_pip")
	IMG("pak::bin\\graphics\\hud\\message box\\icons.png",                                    "Data/Atlases/HUD.xml",                           "game_hud_messagebox_icons")
	IMG("pak::bin\\graphics\\hud\\message box\\next_pip.png",                                 "Data/Atlases/HUD.xml",                           "game_hud_messagebox_pip")
	IMG("pak::bin\\graphics\\hud\\message box\\player_icons.png",                             "Data/Atlases/HUD.xml",                           "game_hud_messagebox_icons")
	IMG("pak::bin\\graphics\\hud\\message bubble\\arrow.png",                                 "Data/Atlases/HUD.xml",                           "game_hud_messagebubble_arrow")
	IMG("pak::bin\\graphics\\hud\\message bubble\\borders.png",                               "Data/Atlases/HUD.xml",                           "game_hud_messagebubble_borders")
	IMG("pak::bin\\graphics\\hud\\message bubble\\old\\arrow.png",                            "Data/Atlases/HUD.xml",                           "game_hud_messagebubble_arrow")
	IMG("pak::bin\\graphics\\hud\\message bubble\\old\\borders.png",                          "Data/Atlases/HUD.xml",                           "game_hud_messagebubble_borders")
	IMG("pak::bin\\graphics\\hud\\ui\\foodvendingmachine\\background.png",                    "Data/Atlases/HUD.xml",                           "game_hud_foodvendingmachine_background")
	IMG("pak::bin\\graphics\\hud\\ui\\foodvendingmachine\\background_backup.png",             "Data/Atlases/HUD.xml",                           "game_hud_foodvendingmachine_backup")
	IMG("pak::bin\\graphics\\hud\\ui\\foodvendingmachine\\cursor.png",                        "Data/Atlases/HUD.xml",                           "game_hud_foodvendingmachine_cursor")
	IMG("pak::bin\\graphics\\hud\\ui\\foodvendingmachine\\foreground.png",                    "Data/Atlases/HUD.xml",                           "game_hud_foodvendingmachine_foreground")
	IMG("pak::bin\\graphics\\hud\\ui\\foodvendingmachine\\highlight_buttons.png",             "Data/Atlases/HUD.xml",                           "game_hud_foodvendingmachine_buttons")
	IMG("pak::bin\\graphics\\hud\\ui\\foodvendingmachine\\items.png",                         "Data/Atlases/HUD.xml",                           "game_hud_foodvendingmachine_items")
	IMG("pak::bin\\graphics\\hud\\ui\\sodavendingmachine\\background.png",                    "Data/Atlases/HUD.xml",                           "game_hud_sodavendingmachine_background")
	IMG("pak::bin\\graphics\\hud\\ui\\sodavendingmachine\\cursor.png",                        "Data/Atlases/HUD.xml",                           "game_hud_sodavendingmachine_cursor")
	IMG("pak::bin\\graphics\\hud\\ui\\sodavendingmachine\\off_pip.png",                       "Data/Atlases/HUD.xml",                           "game_hud_sodavendingmachine_pip")
	IMG("pak::bin\\graphics\\hud\\ui\\sodavendingmachine\\pressed_buttons.png",               "Data/Atlases/HUD.xml",                           "game_hud_sodavendingmachine_buttons")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\acid_gun.png",                                "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_acid_gun")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\ak47.png",                                    "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_ak47")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\ar.png",                                      "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_ar")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\baseball_bat.png",                            "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_baseball_bat")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\bolt_gun.png",                                "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_bolt_gun")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\buff_grenade_ammo.png",                       "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_buff_grenade_ammo")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\buff_grenade_damage.png",                     "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_buff_grenade_damage")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\buff_grenade_health.png",                     "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_buff_grenade_health")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\buff_grenade_price.png",                      "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_buff_grenade_price")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\buff_grenade_reload.png",                     "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_buff_grenade_reload")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\buff_grenade_rof.png",                        "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_buff_grenade_rof")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\buff_grenade_speed.png",                      "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_buff_grenade_speed")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\buff_grenade_xp.png",                         "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_buff_grenade_xp")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\confetti.png",                                "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_confetti")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\doublebarrelshotgun.png",                     "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_doublebarrelshotgun")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\explosive.png",                               "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_explosive")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\fight_saber.png",                             "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_fight_saber")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\flamethrower.png",                            "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_flamethrower")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\flaregun.png",                                "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_flaregun")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\freeze.png",                                  "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_freeze")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\freezeray.png",                               "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_freezeray")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\grenade_launcher.png",                        "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_grenade_launcher")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\incendiary.png",                              "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_incendiary")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\lasercannon.png",                             "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_lasercannon")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\lovecannon.png",                              "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_lovecannon")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\lovethrower.png",                             "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_lovethrower")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\m16.png",                                     "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_m16")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\magnum.png",                                  "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_magnum")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\none.png",                                    "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_none")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\pistol.png",                                  "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_pistol")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\rifle.png",                                   "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_rifle")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\rocket_launcher.png",                         "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_rocket_launcher")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\rpc.png",                                     "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_rpc")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\scattergun.png",                              "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_scattergun")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\shock_rifle.png",                             "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_shock_rifle")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\shotgun.png",                                 "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_shotgun")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\spaz.png",                                    "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_spaz")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\sticky_launcher.png",                         "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_sticky_launcher")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\tripmine.png",                                "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_tripmine")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\turret.png",                                  "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_turret")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\uzi.png",                                     "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_uzi")
	IMG("pak::bin\\graphics\\hud\\weapon icons\\winchester_rifle.png",                        "Data/Atlases/HUD.xml",                           "game_hud_weaponicon_winchester_rifle")
	IMG("pak::bin\\graphics\\items\\paint_bucket.png",                                        "Data/Atlases/Inventory.xml",                     "item_paint_bucket")
	IMG("pak::bin\\graphics\\items\\slots.png",                                               "Data/Atlases/Inventory.xml",                     "item_slots")
	IMG("pak::bin\\graphics\\items\\gems\\ammo capacity\\1.png",                              "Data/Atlases/Inventory.xml",                     "item_gem_ammocapacity_1")
	IMG("pak::bin\\graphics\\items\\gems\\ammo capacity\\2.png",                              "Data/Atlases/Inventory.xml",                     "item_gem_ammocapacity_2")
	IMG("pak::bin\\graphics\\items\\gems\\ammo capacity\\3.png",                              "Data/Atlases/Inventory.xml",                     "item_gem_ammocapacity_3")
	IMG("pak::bin\\graphics\\items\\gems\\ammo capacity\\4.png",                              "Data/Atlases/Inventory.xml",                     "item_gem_ammocapacity_4")
	IMG("pak::bin\\graphics\\items\\gems\\ammo capacity\\5.png",                              "Data/Atlases/Inventory.xml",                     "item_gem_ammocapacity_5")
	IMG("pak::bin\\graphics\\items\\gems\\damage\\1.png",                                     "Data/Atlases/Inventory.xml",                     "item_gem_damage_1")
	IMG("pak::bin\\graphics\\items\\gems\\damage\\2.png",                                     "Data/Atlases/Inventory.xml",                     "item_gem_damage_2")
	IMG("pak::bin\\graphics\\items\\gems\\damage\\3.png",                                     "Data/Atlases/Inventory.xml",                     "item_gem_damage_3")
	IMG("pak::bin\\graphics\\items\\gems\\damage\\4.png",                                     "Data/Atlases/Inventory.xml",                     "item_gem_damage_4")
	IMG("pak::bin\\graphics\\items\\gems\\damage\\5.png",                                     "Data/Atlases/Inventory.xml",                     "item_gem_damage_5")
	IMG("pak::bin\\graphics\\items\\gems\\health\\1.png",                                     "Data/Atlases/Inventory.xml",                     "item_gem_health_1")
	IMG("pak::bin\\graphics\\items\\gems\\health\\2.png",                                     "Data/Atlases/Inventory.xml",                     "item_gem_health_2")
	IMG("pak::bin\\graphics\\items\\gems\\health\\3.png",                                     "Data/Atlases/Inventory.xml",                     "item_gem_health_3")
	IMG("pak::bin\\graphics\\items\\gems\\health\\4.png",                                     "Data/Atlases/Inventory.xml",                     "item_gem_health_4")
	IMG("pak::bin\\graphics\\items\\gems\\health\\5.png",                                     "Data/Atlases/Inventory.xml",                     "item_gem_health_5")
	IMG("pak::bin\\graphics\\items\\gems\\price\\1.png",                                      "Data/Atlases/Inventory.xml",                     "item_gem_price_1")
	IMG("pak::bin\\graphics\\items\\gems\\price\\2.png",                                      "Data/Atlases/Inventory.xml",                     "item_gem_price_2")
	IMG("pak::bin\\graphics\\items\\gems\\price\\3.png",                                      "Data/Atlases/Inventory.xml",                     "item_gem_price_3")
	IMG("pak::bin\\graphics\\items\\gems\\price\\4.png",                                      "Data/Atlases/Inventory.xml",                     "item_gem_price_4")
	IMG("pak::bin\\graphics\\items\\gems\\price\\5.png",                                      "Data/Atlases/Inventory.xml",                     "item_gem_price_5")
	IMG("pak::bin\\graphics\\items\\gems\\rate of fire\\1.png",                               "Data/Atlases/Inventory.xml",                     "item_gem_rof_1")
	IMG("pak::bin\\graphics\\items\\gems\\rate of fire\\2.png",                               "Data/Atlases/Inventory.xml",                     "item_gem_rof_2")
	IMG("pak::bin\\graphics\\items\\gems\\rate of fire\\3.png",                               "Data/Atlases/Inventory.xml",                     "item_gem_rof_3")
	IMG("pak::bin\\graphics\\items\\gems\\rate of fire\\4.png",                               "Data/Atlases/Inventory.xml",                     "item_gem_rof_4")
	IMG("pak::bin\\graphics\\items\\gems\\rate of fire\\5.png",                               "Data/Atlases/Inventory.xml",                     "item_gem_rof_5")
	IMG("pak::bin\\graphics\\items\\gems\\reload speed\\1.png",                               "Data/Atlases/Inventory.xml",                     "item_gem_reload_1")
	IMG("pak::bin\\graphics\\items\\gems\\reload speed\\2.png",                               "Data/Atlases/Inventory.xml",                     "item_gem_reload_2")
	IMG("pak::bin\\graphics\\items\\gems\\reload speed\\3.png",                               "Data/Atlases/Inventory.xml",                     "item_gem_reload_3")
	IMG("pak::bin\\graphics\\items\\gems\\reload speed\\4.png",                               "Data/Atlases/Inventory.xml",                     "item_gem_reload_4")
	IMG("pak::bin\\graphics\\items\\gems\\reload speed\\5.png",                               "Data/Atlases/Inventory.xml",                     "item_gem_reload_5")
	IMG("pak::bin\\graphics\\items\\gems\\speed\\1.png",                                      "Data/Atlases/Inventory.xml",                     "item_gem_speed_1")
	IMG("pak::bin\\graphics\\items\\gems\\speed\\2.png",                                      "Data/Atlases/Inventory.xml",                     "item_gem_speed_2")
	IMG("pak::bin\\graphics\\items\\gems\\speed\\3.png",                                      "Data/Atlases/Inventory.xml",                     "item_gem_speed_3")
	IMG("pak::bin\\graphics\\items\\gems\\speed\\4.png",                                      "Data/Atlases/Inventory.xml",                     "item_gem_speed_4")
	IMG("pak::bin\\graphics\\items\\gems\\speed\\5.png",                                      "Data/Atlases/Inventory.xml",                     "item_gem_speed_5")
	IMG("pak::bin\\graphics\\items\\gems\\xp\\1.png",                                         "Data/Atlases/Inventory.xml",                     "item_gem_xp_1")
	IMG("pak::bin\\graphics\\items\\gems\\xp\\2.png",                                         "Data/Atlases/Inventory.xml",                     "item_gem_xp_2")
	IMG("pak::bin\\graphics\\items\\gems\\xp\\3.png",                                         "Data/Atlases/Inventory.xml",                     "item_gem_xp_3")
	IMG("pak::bin\\graphics\\items\\gems\\xp\\4.png",                                         "Data/Atlases/Inventory.xml",                     "item_gem_xp_4")
	IMG("pak::bin\\graphics\\items\\gems\\xp\\5.png",                                         "Data/Atlases/Inventory.xml",                     "item_gem_xp_5")
	IMG("pak::bin\\graphics\\items\\hats\\ami.png",                                           "Data/Atlases/Inventory.xml",                     "item_hat_ami")
	IMG("pak::bin\\graphics\\items\\hats\\ball_cap.png",                                      "Data/Atlases/Inventory.xml",                     "item_hat_ball_cap")
	IMG("pak::bin\\graphics\\items\\hats\\beanie_1.png",                                      "Data/Atlases/Inventory.xml",                     "item_hat_beanie_1")
	IMG("pak::bin\\graphics\\items\\hats\\beanie_2.png",                                      "Data/Atlases/Inventory.xml",                     "item_hat_beanie_2")
	IMG("pak::bin\\graphics\\items\\hats\\bison_cap.png",                                     "Data/Atlases/Inventory.xml",                     "item_hat_bison_cap")
	IMG("pak::bin\\graphics\\items\\hats\\bison_cap_2.png",                                   "Data/Atlases/Inventory.xml",                     "item_hat_bison_cap_2")
	IMG("pak::bin\\graphics\\items\\hats\\bison_cap_3.png",                                   "Data/Atlases/Inventory.xml",                     "item_hat_bison_cap_3")
	IMG("pak::bin\\graphics\\items\\hats\\box_bot.png",                                       "Data/Atlases/Inventory.xml",                     "item_hat_box_bot")
	IMG("pak::bin\\graphics\\items\\hats\\bsod.png",                                          "Data/Atlases/Inventory.xml",                     "item_hat_bsod")
	IMG("pak::bin\\graphics\\items\\hats\\cheer_bringer.png",                                 "Data/Atlases/Inventory.xml",                     "item_hat_cheer_bringer")
	IMG("pak::bin\\graphics\\items\\hats\\dantz.png",                                         "Data/Atlases/Inventory.xml",                     "item_hat_dantz")
	IMG("pak::bin\\graphics\\items\\hats\\desregardes.png",                                   "Data/Atlases/Inventory.xml",                     "item_hat_desregardes")
	IMG("pak::bin\\graphics\\items\\hats\\desura.png",                                        "Data/Atlases/Inventory.xml",                     "item_hat_desura")
	IMG("pak::bin\\graphics\\items\\hats\\doppleganger.png",                                  "Data/Atlases/Inventory.xml",                     "item_hat_doppleganger")
	IMG("pak::bin\\graphics\\items\\hats\\flat_top.png",                                      "Data/Atlases/Inventory.xml",                     "item_hat_flat_top")
	IMG("pak::bin\\graphics\\items\\hats\\fred_head.png",                                     "Data/Atlases/Inventory.xml",                     "item_hat_fred_head")
	IMG("pak::bin\\graphics\\items\\hats\\gatsby.png",                                        "Data/Atlases/Inventory.xml",                     "item_hat_gatsby")
	IMG("pak::bin\\graphics\\items\\hats\\gi_joe.png",                                        "Data/Atlases/Inventory.xml",                     "item_hat_gi_joe")
	IMG("pak::bin\\graphics\\items\\hats\\golfers_delight.png",                               "Data/Atlases/Inventory.xml",                     "item_hat_golfers_delight")
	IMG("pak::bin\\graphics\\items\\hats\\halo.png",                                          "Data/Atlases/Inventory.xml",                     "item_hat_halo")
	IMG("pak::bin\\graphics\\items\\hats\\heroic_hood.png",                                   "Data/Atlases/Inventory.xml",                     "item_hat_heroic_hood")
	IMG("pak::bin\\graphics\\items\\hats\\little_sis.png",                                    "Data/Atlases/Inventory.xml",                     "item_hat_little_sis")
	IMG("pak::bin\\graphics\\items\\hats\\louisiana_jones.png",                               "Data/Atlases/Inventory.xml",                     "item_hat_louisiana_jones")
	IMG("pak::bin\\graphics\\items\\hats\\mami.png",                                          "Data/Atlases/Inventory.xml",                     "item_hat_mami")
	IMG("pak::bin\\graphics\\items\\hats\\nekomimi.png",                                      "Data/Atlases/Inventory.xml",                     "item_hat_nekomimi")
	IMG("pak::bin\\graphics\\items\\hats\\p1_hair.png",                                       "Data/Atlases/Inventory.xml",                     "item_hat_p1_hair")
	IMG("pak::bin\\graphics\\items\\hats\\p2_hair.png",                                       "Data/Atlases/Inventory.xml",                     "item_hat_p2_hair")
	IMG("pak::bin\\graphics\\items\\hats\\p3_hair.png",                                       "Data/Atlases/Inventory.xml",                     "item_hat_p3_hair")
	IMG("pak::bin\\graphics\\items\\hats\\p4_hair.png",                                       "Data/Atlases/Inventory.xml",                     "item_hat_p4_hair")
	IMG("pak::bin\\graphics\\items\\hats\\pigtails.png",                                      "Data/Atlases/Inventory.xml",                     "item_hat_pigtails")
	IMG("pak::bin\\graphics\\items\\hats\\pillager.png",                                      "Data/Atlases/Inventory.xml",                     "item_hat_pillager")
	IMG("pak::bin\\graphics\\items\\hats\\pompador.png",                                      "Data/Atlases/Inventory.xml",                     "item_hat_pompador")
	IMG("pak::bin\\graphics\\items\\hats\\poofy.png",                                         "Data/Atlases/Inventory.xml",                     "item_hat_poofy")
	IMG("pak::bin\\graphics\\items\\hats\\presento.png",                                      "Data/Atlases/Inventory.xml",                     "item_hat_presento")
	IMG("pak::bin\\graphics\\items\\hats\\pumpkin.png",                                       "Data/Atlases/Inventory.xml",                     "item_hat_pumpkin")
	IMG("pak::bin\\graphics\\items\\hats\\sgt_masterson.png",                                 "Data/Atlases/Inventory.xml",                     "item_hat_sgt_masterson")
	IMG("pak::bin\\graphics\\items\\hats\\shizue.png",                                        "Data/Atlases/Inventory.xml",                     "item_hat_shizue")
	IMG("pak::bin\\graphics\\items\\hats\\sidekick.png",                                      "Data/Atlases/Inventory.xml",                     "item_hat_sidekick")
	IMG("pak::bin\\graphics\\items\\hats\\skull.png",                                         "Data/Atlases/Inventory.xml",                     "item_hat_skull")
	IMG("pak::bin\\graphics\\items\\hats\\steve.png",                                         "Data/Atlases/Inventory.xml",                     "item_hat_steve")
	IMG("pak::bin\\graphics\\items\\hats\\thug_pomp.png",                                     "Data/Atlases/Inventory.xml",                     "item_hat_thug_pomp")
	IMG("pak::bin\\graphics\\items\\hats\\top_hat.png",                                       "Data/Atlases/Inventory.xml",                     "item_hat_top_hat")
	IMG("pak::bin\\graphics\\items\\hats\\trucker.png",                                       "Data/Atlases/Inventory.xml",                     "item_hat_trucker")
	IMG("pak::bin\\graphics\\items\\hats\\turbo_boy.png",                                     "Data/Atlases/Inventory.xml",                     "item_hat_turbo_boy")
	IMG("pak::bin\\graphics\\items\\hats\\twindrills.png",                                    "Data/Atlases/Inventory.xml",                     "item_hat_twindrills")
	IMG("pak::bin\\graphics\\items\\hats\\twintails.png",                                     "Data/Atlases/Inventory.xml",                     "item_hat_twintails")
	IMG("pak::bin\\graphics\\items\\hats\\yayoi.png",                                         "Data/Atlases/Inventory.xml",                     "item_hat_yayoi")
	IMG("pak::bin\\graphics\\items\\misc\\10000_coins.png",                                   "Data/Atlases/Inventory.xml",                     "item_accessory_10000_coins")
	IMG("pak::bin\\graphics\\items\\misc\\1000_coins.png",                                    "Data/Atlases/Inventory.xml",                     "item_accessory_1000_coins")
	IMG("pak::bin\\graphics\\items\\misc\\2500_coins.png",                                    "Data/Atlases/Inventory.xml",                     "item_accessory_2500_coins")
	IMG("pak::bin\\graphics\\items\\misc\\antlers.png",                                       "Data/Atlases/Inventory.xml",                     "item_accessory_antlers")
	IMG("pak::bin\\graphics\\items\\misc\\armour.png",                                        "Data/Atlases/Inventory.xml",                     "item_accessory_armour")
	IMG("pak::bin\\graphics\\items\\misc\\aviators.png",                                      "Data/Atlases/Inventory.xml",                     "item_accessory_aviators")
	IMG("pak::bin\\graphics\\items\\misc\\bow.png",                                           "Data/Atlases/Inventory.xml",                     "item_accessory_bow")
	IMG("pak::bin\\graphics\\items\\misc\\deus ex.png",                                       "Data/Atlases/Inventory.xml",                     "item_accessory_deus ex")
	IMG("pak::bin\\graphics\\items\\misc\\gameboy.png",                                       "Data/Atlases/Inventory.xml",                     "item_accessory_gameboy")
	IMG("pak::bin\\graphics\\items\\misc\\ghetto_superstar.png",                              "Data/Atlases/Inventory.xml",                     "item_accessory_ghetto_superstar")
	IMG("pak::bin\\graphics\\items\\misc\\glasses.png",                                       "Data/Atlases/Inventory.xml",                     "item_accessory_glasses")
	IMG("pak::bin\\graphics\\items\\misc\\headset.png",                                       "Data/Atlases/Inventory.xml",                     "item_accessory_headset")
	IMG("pak::bin\\graphics\\items\\misc\\scouter.png",                                       "Data/Atlases/Inventory.xml",                     "item_accessory_scouter")
	IMG("pak::bin\\graphics\\items\\misc\\shades.png",                                        "Data/Atlases/Inventory.xml",                     "item_accessory_shades")
	IMG("pak::bin\\graphics\\items\\premium\\add_gem_slot.png",                               "Data/Atlases/Inventory.xml",                     "item_premium_add_gem_slot")
	IMG("pak::bin\\graphics\\items\\premium\\gem_grab_bag.png",                               "Data/Atlases/Inventory.xml",                     "item_premium_gem_grab_bag")
	IMG("pak::bin\\graphics\\items\\premium\\gem_removal_kit.png",                            "Data/Atlases/Inventory.xml",                     "item_premium_gem_removal_kit")
	IMG("pak::bin\\graphics\\items\\premium\\large_donator.png",                              "Data/Atlases/Inventory.xml",                     "item_premium_large_donator")
	IMG("pak::bin\\graphics\\items\\premium\\small_donator.png",                              "Data/Atlases/Inventory.xml",                     "item_premium_small_donator")
	IMG("pak::bin\\graphics\\items\\weapons\\acid_gun.png",                                   "Data/Atlases/Inventory.xml",                     "item_weapon_acid_gun")
	IMG("pak::bin\\graphics\\items\\weapons\\ak47.png",                                       "Data/Atlases/Inventory.xml",                     "item_weapon_ak47")
	IMG("pak::bin\\graphics\\items\\weapons\\ammo_dispenser.png",                             "Data/Atlases/Inventory.xml",                     "item_weapon_ammo_dispenser")
	IMG("pak::bin\\graphics\\items\\weapons\\baseball bat.png",                               "Data/Atlases/Inventory.xml",                     "item_weapon_baseball bat")
	IMG("pak::bin\\graphics\\items\\weapons\\bolt_gun.png",                                   "Data/Atlases/Inventory.xml",                     "item_weapon_bolt_gun")
	IMG("pak::bin\\graphics\\items\\weapons\\buff_grenade_ammo.png",                          "Data/Atlases/Inventory.xml",                     "item_weapon_buff_grenade_ammo")
	IMG("pak::bin\\graphics\\items\\weapons\\buff_grenade_damage.png",                        "Data/Atlases/Inventory.xml",                     "item_weapon_buff_grenade_damage")
	IMG("pak::bin\\graphics\\items\\weapons\\buff_grenade_health.png",                        "Data/Atlases/Inventory.xml",                     "item_weapon_buff_grenade_health")
	IMG("pak::bin\\graphics\\items\\weapons\\buff_grenade_price.png",                         "Data/Atlases/Inventory.xml",                     "item_weapon_buff_grenade_price")
	IMG("pak::bin\\graphics\\items\\weapons\\buff_grenade_reload.png",                        "Data/Atlases/Inventory.xml",                     "item_weapon_buff_grenade_reload")
	IMG("pak::bin\\graphics\\items\\weapons\\buff_grenade_rof.png",                           "Data/Atlases/Inventory.xml",                     "item_weapon_buff_grenade_rof")
	IMG("pak::bin\\graphics\\items\\weapons\\buff_grenade_speed.png",                         "Data/Atlases/Inventory.xml",                     "item_weapon_buff_grenade_speed")
	IMG("pak::bin\\graphics\\items\\weapons\\buff_grenade_xp.png",                            "Data/Atlases/Inventory.xml",                     "item_weapon_buff_grenade_xp")
	IMG("pak::bin\\graphics\\items\\weapons\\confettiammo.png",                               "Data/Atlases/Inventory.xml",                     "item_weapon_confettiammo")
	IMG("pak::bin\\graphics\\items\\weapons\\double_barrel_shotgun.png",                      "Data/Atlases/Inventory.xml",                     "item_weapon_double_barrel_shotgun")
	IMG("pak::bin\\graphics\\items\\weapons\\explosiveammo.png",                              "Data/Atlases/Inventory.xml",                     "item_weapon_explosiveammo")
	IMG("pak::bin\\graphics\\items\\weapons\\fight_saber.png",                                "Data/Atlases/Inventory.xml",                     "item_weapon_fight_saber")
	IMG("pak::bin\\graphics\\items\\weapons\\fire turret.png",                                "Data/Atlases/Inventory.xml",                     "item_weapon_fire turret")
	IMG("pak::bin\\graphics\\items\\weapons\\flamethrower.png",                               "Data/Atlases/Inventory.xml",                     "item_weapon_flamethrower")
	IMG("pak::bin\\graphics\\items\\weapons\\flare_gun.png",                                  "Data/Atlases/Inventory.xml",                     "item_weapon_flare_gun")
	IMG("pak::bin\\graphics\\items\\weapons\\freezeammo.png",                                 "Data/Atlases/Inventory.xml",                     "item_weapon_freezeammo")
	IMG("pak::bin\\graphics\\items\\weapons\\freeze_ray.png",                                 "Data/Atlases/Inventory.xml",                     "item_weapon_freeze_ray")
	IMG("pak::bin\\graphics\\items\\weapons\\grenade_launcher.png",                           "Data/Atlases/Inventory.xml",                     "item_weapon_grenade_launcher")
	IMG("pak::bin\\graphics\\items\\weapons\\health_dispenser.png",                           "Data/Atlases/Inventory.xml",                     "item_weapon_health_dispenser")
	IMG("pak::bin\\graphics\\items\\weapons\\incendiaryammo.png",                             "Data/Atlases/Inventory.xml",                     "item_weapon_incendiaryammo")
	IMG("pak::bin\\graphics\\items\\weapons\\laser_cannon.png",                               "Data/Atlases/Inventory.xml",                     "item_weapon_laser_cannon")
	IMG("pak::bin\\graphics\\items\\weapons\\love_cannon.png",                                "Data/Atlases/Inventory.xml",                     "item_weapon_love_cannon")
	IMG("pak::bin\\graphics\\items\\weapons\\love_thrower.png",                               "Data/Atlases/Inventory.xml",                     "item_weapon_love_thrower")
	IMG("pak::bin\\graphics\\items\\weapons\\m16.png",                                        "Data/Atlases/Inventory.xml",                     "item_weapon_m16")
	IMG("pak::bin\\graphics\\items\\weapons\\magnum.png",                                     "Data/Atlases/Inventory.xml",                     "item_weapon_magnum")
	IMG("pak::bin\\graphics\\items\\weapons\\pistol.png",                                     "Data/Atlases/Inventory.xml",                     "item_weapon_pistol")
	IMG("pak::bin\\graphics\\items\\weapons\\rocket turret.png",                              "Data/Atlases/Inventory.xml",                     "item_weapon_rocket turret")
	IMG("pak::bin\\graphics\\items\\weapons\\rocket_launcher.png",                            "Data/Atlases/Inventory.xml",                     "item_weapon_rocket_launcher")
	IMG("pak::bin\\graphics\\items\\weapons\\rpc.png",                                        "Data/Atlases/Inventory.xml",                     "item_weapon_rpc")
	IMG("pak::bin\\graphics\\items\\weapons\\scatter_gun.png",                                "Data/Atlases/Inventory.xml",                     "item_weapon_scatter_gun")
	IMG("pak::bin\\graphics\\items\\weapons\\shock_rifle.png",                                "Data/Atlases/Inventory.xml",                     "item_weapon_shock_rifle")
	IMG("pak::bin\\graphics\\items\\weapons\\shotgun.png",                                    "Data/Atlases/Inventory.xml",                     "item_weapon_shotgun")
	IMG("pak::bin\\graphics\\items\\weapons\\spaz.png",                                       "Data/Atlases/Inventory.xml",                     "item_weapon_spaz")
	IMG("pak::bin\\graphics\\items\\weapons\\sticky_launcher.png",                            "Data/Atlases/Inventory.xml",                     "item_weapon_sticky_launcher")
	IMG("pak::bin\\graphics\\items\\weapons\\tripmine.png",                                   "Data/Atlases/Inventory.xml",                     "item_weapon_tripmine")
	IMG("pak::bin\\graphics\\items\\weapons\\turret.png",                                     "Data/Atlases/Inventory.xml",                     "item_weapon_turret")
	IMG("pak::bin\\graphics\\items\\weapons\\uzi.png",                                        "Data/Atlases/Inventory.xml",                     "item_weapon_uzi")
	IMG("pak::bin\\graphics\\items\\weapons\\winchester_rifle.png",                           "Data/Atlases/Inventory.xml",                     "item_weapon_winchester_rifle")
	IMG("pak::bin\\graphics\\menus\\credits\\end.png",                                        "Data/Atlases/Screens.xml",                       "screen_credits_end")
	IMG("pak::bin\\graphics\\menus\\intro\\0.png",                                            "Data/Atlases/Screens.xml",                       "screen_intro_0")
	IMG("pak::bin\\graphics\\menus\\intro\\1.png",                                            "Data/Atlases/Screens.xml",                       "screen_intro_1")
	IMG("pak::bin\\graphics\\menus\\intro\\2.png",                                            "Data/Atlases/Screens.xml",                       "screen_intro_2")
	IMG("pak::bin\\graphics\\menus\\intro\\3.png",                                            "Data/Atlases/Screens.xml",                       "screen_intro_3")
	IMG("pak::bin\\graphics\\menus\\loading\\loading_1.png",                                  "Data/Atlases/Screens.xml",                       "screen_loading_loading_1")
	IMG("pak::bin\\graphics\\menus\\loading\\loading_2.png",                                  "Data/Atlases/Screens.xml",                       "screen_loading_loading_2")
	IMG("pak::bin\\graphics\\menus\\loading\\loading_3.png",                                  "Data/Atlases/Screens.xml",                       "screen_loading_loading_3")
	IMG("pak::bin\\graphics\\menus\\loading\\loading_4.png",                                  "Data/Atlases/Screens.xml",                       "screen_loading_loading_4")
	IMG("pak::bin\\graphics\\menus\\loading\\person_1.png",                                   "Data/Atlases/Screens.xml",                       "screen_loading_person_1")
	IMG("pak::bin\\graphics\\menus\\loading\\person_2.png",                                   "Data/Atlases/Screens.xml",                       "screen_loading_person_2")
	IMG("pak::bin\\graphics\\menus\\loading\\person_3.png",                                   "Data/Atlases/Screens.xml",                       "screen_loading_person_3")
	IMG("pak::bin\\graphics\\menus\\loading\\person_4.png",                                   "Data/Atlases/Screens.xml",                       "screen_loading_person_4")
	IMG("pak::bin\\graphics\\menus\\main\\arrows.png",                                        "Data/Atlases/Screens.xml",                       "screen_main_arrows")
	IMG("pak::bin\\graphics\\menus\\main\\bg_1.png",                                          "Data/Atlases/Screens.xml",                       "screen_main_bg_1")
	IMG("pak::bin\\graphics\\menus\\main\\box_active.png",                                    "Data/Atlases/Screens.xml",                       "screen_main_box_active")
	IMG("pak::bin\\graphics\\menus\\main\\box_inactive.png",                                  "Data/Atlases/Screens.xml",                       "screen_main_box_inactive")
	IMG("pak::bin\\graphics\\menus\\main\\box_tintable.png",                                  "Data/Atlases/Screens.xml",                       "screen_main_box_tintable")
	IMG("pak::bin\\graphics\\menus\\main\\button.png",                                        "Data/Atlases/Screens.xml",                       "screen_main_button")
	IMG("pak::bin\\graphics\\menus\\main\\button_active.png",                                 "Data/Atlases/Screens.xml",                       "screen_main_button_active")
	IMG("pak::bin\\graphics\\menus\\main\\button_press.png",                                  "Data/Atlases/Screens.xml",                       "screen_main_button_press")
	IMG("pak::bin\\graphics\\menus\\main\\combo_box.png",                                     "Data/Atlases/Screens.xml",                       "screen_main_combo_box")
	IMG("pak::bin\\graphics\\menus\\main\\cursor.png",                                        "Data/Atlases/Screens.xml",                       "screen_main_cursor")
	IMG("pak::bin\\graphics\\menus\\main\\input_active.png",                                  "Data/Atlases/Screens.xml",                       "screen_main_input_active")
	IMG("pak::bin\\graphics\\menus\\main\\input_inactive.png",                                "Data/Atlases/Screens.xml",                       "screen_main_input_inactive")
	IMG("pak::bin\\graphics\\menus\\main\\map_loading.png",                                   "Data/Atlases/Screens.xml",                       "screen_main_map_loading")
	IMG("pak::bin\\graphics\\menus\\main\\map_preview.png",                                   "Data/Atlases/Screens.xml",                       "screen_main_map_preview")
	IMG("pak::bin\\graphics\\menus\\main\\map_preview_foreground.png",                        "Data/Atlases/Screens.xml",                       "screen_main_map_preview_foreground")
	IMG("pak::bin\\graphics\\menus\\main\\plus_button.png",                                   "Data/Atlases/Screens.xml",                       "screen_main_plus_button")
	IMG("pak::bin\\graphics\\menus\\main\\progress_border.png",                               "Data/Atlases/Screens.xml",                       "screen_main_progress_border")
	IMG("pak::bin\\graphics\\menus\\main\\radio_button.png",                                  "Data/Atlases/Screens.xml",                       "screen_main_radio_button")
	IMG("pak::bin\\graphics\\menus\\main\\slider.png",                                        "Data/Atlases/Screens.xml",                       "screen_main_slider")
	IMG("pak::bin\\graphics\\menus\\main\\tickbox.png",                                       "Data/Atlases/Screens.xml",                       "screen_main_tickbox")
	IMG("pak::bin\\graphics\\menus\\main\\title.png",                                         "Data/Atlases/Screens.xml",                       "screen_main_title")
	IMG("pak::bin\\graphics\\menus\\sub\\background.png",                                     "Data/Atlases/Screens.xml",                       "screen_sub_background")
	IMG("pak::bin\\graphics\\normal maps\\heatshimmer.png",                                   "Data/Atlases/Distortion.xml",                    "distortion_heatshimmer")
	IMG("pak::bin\\graphics\\normal maps\\shockwave.png",                                     "Data/Atlases/Distortion.xml",                    "distortion_shockwave")
	IMG("pak::bin\\graphics\\normal maps\\small_projectile.png",                              "Data/Atlases/Distortion.xml",                    "distortion_small_projectile")
	IMG("pak::bin\\graphics\\normal maps\\small_projectile_streak.png",                       "Data/Atlases/Distortion.xml",                    "distortion_small_projectile_streak")
	IMG("pak::bin\\graphics\\objects\\ammo crate\\default.png",                               "Data/Atlases/Actors.xml",                        "actor_ammo_crate_default")
	IMG("pak::bin\\graphics\\objects\\avatars\\blood.png",                                    "Data/Atlases/Avatar.xml",                        "avatar_blood")
	IMG("pak::bin\\graphics\\objects\\avatars\\flashlight.png",                               "Data/Atlases/Avatar.xml",                        "avatar_flashlight")
	IMG("pak::bin\\graphics\\objects\\avatars\\ghost.png",                                    "Data/Atlases/Avatar.xml",                        "avatar_ghost")
	IMG("pak::bin\\graphics\\objects\\avatars\\grave.png",                                    "Data/Atlases/Avatar.xml",                        "avatar_grave")
	IMG("pak::bin\\graphics\\objects\\avatars\\spawn_bottom.png",                             "Data/Atlases/Avatar.xml",                        "avatar_spawn_bottom")
	IMG("pak::bin\\graphics\\objects\\avatars\\spawn_top.png",                                "Data/Atlases/Avatar.xml",                        "avatar_spawn_top")
	IMG("pak::bin\\graphics\\objects\\avatars\\splat.png",                                    "Data/Atlases/Avatar.xml",                        "avatar_splat")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\acid_gun.png",                     "Data/Atlases/Avatar.xml",                        "avatar_body_acid_gun")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\ak47.png",                         "Data/Atlases/Avatar.xml",                        "avatar_body_ak47")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\ammocapacity_grenade.png",         "Data/Atlases/Avatar.xml",                        "avatar_body_ammocapacity_grenade")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\baseballbat.png",                  "Data/Atlases/Avatar.xml",                        "avatar_body_baseballbat")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\bolt_gun.png",                     "Data/Atlases/Avatar.xml",                        "avatar_body_bolt_gun")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\damage_grenade.png",               "Data/Atlases/Avatar.xml",                        "avatar_body_damage_grenade")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\dbshotgun.png",                    "Data/Atlases/Avatar.xml",                        "avatar_body_dbshotgun")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\fight_saber.png",                  "Data/Atlases/Avatar.xml",                        "avatar_body_fight_saber")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\flamethrower.png",                 "Data/Atlases/Avatar.xml",                        "avatar_body_flamethrower")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\flaregun.png",                     "Data/Atlases/Avatar.xml",                        "avatar_body_flaregun")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\freezeray.png",                    "Data/Atlases/Avatar.xml",                        "avatar_body_freezeray")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\grenadelauncher.png",              "Data/Atlases/Avatar.xml",                        "avatar_body_grenadelauncher")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\health_grenade.png",               "Data/Atlases/Avatar.xml",                        "avatar_body_health_grenade")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\idle.png",                         "Data/Atlases/Avatar.xml",                        "avatar_body_idle")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\lasercannon.png",                  "Data/Atlases/Avatar.xml",                        "avatar_body_lasercannon")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\lovecannon.png",                   "Data/Atlases/Avatar.xml",                        "avatar_body_lovecannon")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\love_thrower.png",                 "Data/Atlases/Avatar.xml",                        "avatar_body_love_thrower")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\m16.png",                          "Data/Atlases/Avatar.xml",                        "avatar_body_m16")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\magnum.png",                       "Data/Atlases/Avatar.xml",                        "avatar_body_magnum")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\pistol.png",                       "Data/Atlases/Avatar.xml",                        "avatar_body_pistol")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\price_grenade.png",                "Data/Atlases/Avatar.xml",                        "avatar_body_price_grenade")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\reload_grenade.png",               "Data/Atlases/Avatar.xml",                        "avatar_body_reload_grenade")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\rocketlauncher.png",               "Data/Atlases/Avatar.xml",                        "avatar_body_rocketlauncher")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\rof_grenade.png",                  "Data/Atlases/Avatar.xml",                        "avatar_body_rof_grenade")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\rpc.png",                          "Data/Atlases/Avatar.xml",                        "avatar_body_rpc")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\shock_rifle.png",                  "Data/Atlases/Avatar.xml",                        "avatar_body_shock_rifle")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\shotgun.png",                      "Data/Atlases/Avatar.xml",                        "avatar_body_shotgun")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\spaz.png",                         "Data/Atlases/Avatar.xml",                        "avatar_body_spaz")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\speed_grenade.png",                "Data/Atlases/Avatar.xml",                        "avatar_body_speed_grenade")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\spread.png",                       "Data/Atlases/Avatar.xml",                        "avatar_body_spread")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\stickylauncher.png",               "Data/Atlases/Avatar.xml",                        "avatar_body_stickylauncher")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\uzi.png",                          "Data/Atlases/Avatar.xml",                        "avatar_body_uzi")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\winchester_rifle.png",             "Data/Atlases/Avatar.xml",                        "avatar_body_winchester_rifle")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\body\\xp_grenade.png",                   "Data/Atlases/Avatar.xml",                        "avatar_body_xp_grenade")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\gib\\gibs.png",                          "Data/Atlases/Avatar.xml",                        "avatar_gib_gibs")
	IMG("pak::bin\\graphics\\objects\\avatars\\body\\leg\\legs.png",                          "Data/Atlases/Avatar.xml",                        "avatar_leg_legs")
	IMG("pak::bin\\graphics\\objects\\barricades\\human\\barricade_1.png",                    "Data/Atlases/Actors.xml",                        "actor_barricade_human_barricade_1")
	IMG("pak::bin\\graphics\\objects\\barricades\\human\\barricade_2.png",                    "Data/Atlases/Actors.xml",                        "actor_barricade_human_barricade_2")
	IMG("pak::bin\\graphics\\objects\\barricades\\human\\barricade_horizontal_1.png",         "Data/Atlases/Actors.xml",                        "actor_barricade_human_barricade_horizontal_1")
	IMG("pak::bin\\graphics\\objects\\barricades\\human\\barricade_vertical_1.png",           "Data/Atlases/Actors.xml",                        "actor_barricade_human_barricade_vertical_1")
	IMG("pak::bin\\graphics\\objects\\barricades\\human\\box_1.png",                          "Data/Atlases/Actors.xml",                        "actor_barricade_human_box_1")
	IMG("pak::bin\\graphics\\objects\\barricades\\human\\box_2.png",                          "Data/Atlases/Actors.xml",                        "actor_barricade_human_box_2")
	IMG("pak::bin\\graphics\\objects\\barricades\\human\\metal_box_1.png",                    "Data/Atlases/Actors.xml",                        "actor_barricade_human_metal_box_1")
	IMG("pak::bin\\graphics\\objects\\barricades\\human\\metal_box_2.png",                    "Data/Atlases/Actors.xml",                        "actor_barricade_human_metal_box_2")
	IMG("pak::bin\\graphics\\objects\\barricades\\human\\puff.png",                           "Data/Atlases/Actors.xml",                        "actor_barricade_human_puff")
	IMG("pak::bin\\graphics\\objects\\barricades\\zombie\\barricade_1.png",                   "Data/Atlases/Actors.xml",                        "actor_barricade_zombie_barricade_1")
	IMG("pak::bin\\graphics\\objects\\barricades\\zombie\\barricade_2.png",                   "Data/Atlases/Actors.xml",                        "actor_barricade_zombie_barricade_2")
	IMG("pak::bin\\graphics\\objects\\barricades\\zombie\\barricade_3.png",                   "Data/Atlases/Actors.xml",                        "actor_barricade_zombie_barricade_3")
	IMG("pak::bin\\graphics\\objects\\chests\\special.png",                                   "Data/Atlases/Actors.xml",                        "actor_chest_special")
	IMG("pak::bin\\graphics\\objects\\chests\\standard.png",                                  "Data/Atlases/Actors.xml",                        "actor_chest_standard")
	IMG("pak::bin\\graphics\\objects\\cutscene\\camera.png",                                  "Data/Atlases/Actors.xml",                        "actor_cutscene_camera")
	IMG("pak::bin\\graphics\\objects\\damage zone\\default.png",                              "Data/Atlases/Actors.xml",                        "actor_damage_zone_default")
	IMG("pak::bin\\graphics\\objects\\decal\\default.png",                                    "Data/Atlases/Decals.xml",                        "decal_default")
	IMG("pak::bin\\graphics\\objects\\decal\\insidemapmarker.png",                            "Data/Atlases/Decals.xml",                        "decal_inside_insidemapmarker")
	IMG("pak::bin\\graphics\\objects\\decal\\outsidemapmarker.png",                           "Data/Atlases/Decals.xml",                        "decal_outsidemapmarker")
	IMG("pak::bin\\graphics\\objects\\decal\\barrels\\all_colors.png",                        "Data/Atlases/Decals.xml",                        "decal_barrel_all_colors")
	IMG("pak::bin\\graphics\\objects\\decal\\barrels\\all_colors_side.png",                   "Data/Atlases/Decals.xml",                        "decal_barrel_all_colors_side")
	IMG("pak::bin\\graphics\\objects\\decal\\barrels\\blue.png",                              "Data/Atlases/Decals.xml",                        "decal_barrel_blue")
	IMG("pak::bin\\graphics\\objects\\decal\\barrels\\brown.png",                             "Data/Atlases/Decals.xml",                        "decal_barrel_brown")
	IMG("pak::bin\\graphics\\objects\\decal\\barrels\\green.png",                             "Data/Atlases/Decals.xml",                        "decal_barrel_green")
	IMG("pak::bin\\graphics\\objects\\decal\\barrels\\red.png",                               "Data/Atlases/Decals.xml",                        "decal_barrel_red")
	IMG("pak::bin\\graphics\\objects\\decal\\barricades\\horizontal.png",                     "Data/Atlases/Decals.xml",                        "decal_barricade_horizontal")
	IMG("pak::bin\\graphics\\objects\\decal\\barricades\\nw_se.png",                          "Data/Atlases/Decals.xml",                        "decal_barricade_nw_se")
	IMG("pak::bin\\graphics\\objects\\decal\\barricades\\sw_ne.png",                          "Data/Atlases/Decals.xml",                        "decal_barricade_sw_ne")
	IMG("pak::bin\\graphics\\objects\\decal\\barricades\\vertical.png",                       "Data/Atlases/Decals.xml",                        "decal_barricade_vertical")
	IMG("pak::bin\\graphics\\objects\\decal\\bins\\dumpster_brown.png",                       "Data/Atlases/Decals.xml",                        "decal_bin_dumpster_brown")
	IMG("pak::bin\\graphics\\objects\\decal\\bins\\dumpster_green.png",                       "Data/Atlases/Decals.xml",                        "decal_bin_dumpster_green")
	IMG("pak::bin\\graphics\\objects\\decal\\bins\\gold.png",                                 "Data/Atlases/Decals.xml",                        "decal_bin_gold")
	IMG("pak::bin\\graphics\\objects\\decal\\bins\\green.png",                                "Data/Atlases/Decals.xml",                        "decal_bin_green")
	IMG("pak::bin\\graphics\\objects\\decal\\bins\\silver.png",                               "Data/Atlases/Decals.xml",                        "decal_bin_silver")
	IMG("pak::bin\\graphics\\objects\\decal\\blood\\blood_1.png",                             "Data/Atlases/Decals.xml",                        "decal_blood_blood_1")
	IMG("pak::bin\\graphics\\objects\\decal\\blood\\blood_2.png",                             "Data/Atlases/Decals.xml",                        "decal_blood_blood_2")
	IMG("pak::bin\\graphics\\objects\\decal\\bushs\\bush_1.png",                              "Data/Atlases/Decals.xml",                        "decal_bush_bush_1")
	IMG("pak::bin\\graphics\\objects\\decal\\bushs\\bush_2.png",                              "Data/Atlases/Decals.xml",                        "decal_bush_bush_2")
	IMG("pak::bin\\graphics\\objects\\decal\\bushs\\corner.png",                              "Data/Atlases/Decals.xml",                        "decal_bush_corner")
	IMG("pak::bin\\graphics\\objects\\decal\\bushs\\dead_stump.png",                          "Data/Atlases/Decals.xml",                        "decal_bush_dead_stump")
	IMG("pak::bin\\graphics\\objects\\decal\\bushs\\dead_tree1.png",                          "Data/Atlases/Decals.xml",                        "decal_bush_dead_tree1")
	IMG("pak::bin\\graphics\\objects\\decal\\bushs\\horizontal.png",                          "Data/Atlases/Decals.xml",                        "decal_bush_horizontal")
	IMG("pak::bin\\graphics\\objects\\decal\\bushs\\tall_tree.png",                           "Data/Atlases/Decals.xml",                        "decal_bush_tall_tree")
	IMG("pak::bin\\graphics\\objects\\decal\\bushs\\tiled_tree.png",                          "Data/Atlases/Decals.xml",                        "decal_bush_tiled_tree")
	IMG("pak::bin\\graphics\\objects\\decal\\bushs\\tree.png",                                "Data/Atlases/Decals.xml",                        "decal_bush_tree")
	IMG("pak::bin\\graphics\\objects\\decal\\bushs\\vertical.png",                            "Data/Atlases/Decals.xml",                        "decal_bush_vertical")
	IMG("pak::bin\\graphics\\objects\\decal\\bushs\\vines.png",                               "Data/Atlases/Decals.xml",                        "decal_bush_vines")
	IMG("pak::bin\\graphics\\objects\\decal\\bushs\\zg_bushes.png",                           "Data/Atlases/Decals.xml",                        "decal_bush_zg_bushes")
	IMG("pak::bin\\graphics\\objects\\decal\\crates\\crate_1.png",                            "Data/Atlases/Decals.xml",                        "decal_crate_crate_1")
	IMG("pak::bin\\graphics\\objects\\decal\\crates\\crate_2.png",                            "Data/Atlases/Decals.xml",                        "decal_crate_crate_2")
	IMG("pak::bin\\graphics\\objects\\decal\\crates\\crate_3.png",                            "Data/Atlases/Decals.xml",                        "decal_crate_crate_3")
	IMG("pak::bin\\graphics\\objects\\decal\\crates\\crate_4.png",                            "Data/Atlases/Decals.xml",                        "decal_crate_crate_4")
	IMG("pak::bin\\graphics\\objects\\decal\\crates\\crate_5.png",                            "Data/Atlases/Decals.xml",                        "decal_crate_crate_5")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\block_1.png",                             "Data/Atlases/Decals.xml",                        "decal_egypt_block_1")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\block_2.png",                             "Data/Atlases/Decals.xml",                        "decal_egypt_block_2")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\egyptian_brix1.png",                      "Data/Atlases/Decals.xml",                        "decal_egypt_egyptian_brix1")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\egyptian_brix2.png",                      "Data/Atlases/Decals.xml",                        "decal_egypt_egyptian_brix2")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\grass_1.png",                             "Data/Atlases/Decals.xml",                        "decal_egypt_grass_1")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\grass_2.png",                             "Data/Atlases/Decals.xml",                        "decal_egypt_grass_2")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\grass_3.png",                             "Data/Atlases/Decals.xml",                        "decal_egypt_grass_3")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\pillar1.png",                             "Data/Atlases/Decals.xml",                        "decal_egypt_pillar1")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\pillar2.png",                             "Data/Atlases/Decals.xml",                        "decal_egypt_pillar2")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\pillar3.png",                             "Data/Atlases/Decals.xml",                        "decal_egypt_pillar3")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\pillar4.png",                             "Data/Atlases/Decals.xml",                        "decal_egypt_pillar4")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\pillar5.png",                             "Data/Atlases/Decals.xml",                        "decal_egypt_pillar5")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\pillar_down1.png",                        "Data/Atlases/Decals.xml",                        "decal_egypt_pillar_down1")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\pillar_down2.png",                        "Data/Atlases/Decals.xml",                        "decal_egypt_pillar_down2")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\pillar_down3.png",                        "Data/Atlases/Decals.xml",                        "decal_egypt_pillar_down3")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\pillar_down4.png",                        "Data/Atlases/Decals.xml",                        "decal_egypt_pillar_down4")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\pillar_down5.png",                        "Data/Atlases/Decals.xml",                        "decal_egypt_pillar_down5")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\pillar_top.png",                          "Data/Atlases/Decals.xml",                        "decal_egypt_pillar_top")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\wall_decal1.png",                         "Data/Atlases/Decals.xml",                        "decal_egypt_wall_decal1")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\wall_decal2.png",                         "Data/Atlases/Decals.xml",                        "decal_egypt_wall_decal2")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\wall_decal3.png",                         "Data/Atlases/Decals.xml",                        "decal_egypt_wall_decal3")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\wall_decal4.png",                         "Data/Atlases/Decals.xml",                        "decal_egypt_wall_decal4")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\wall_decal5.png",                         "Data/Atlases/Decals.xml",                        "decal_egypt_wall_decal5")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\wall_decal6.png",                         "Data/Atlases/Decals.xml",                        "decal_egypt_wall_decal6")
	IMG("pak::bin\\graphics\\objects\\decal\\egypt\\wall_decal7.png",                         "Data/Atlases/Decals.xml",                        "decal_egypt_wall_decal7")
	IMG("pak::bin\\graphics\\objects\\decal\\fences\\horizontal_fence_tile.png",              "Data/Atlases/Decals.xml",                        "decal_fence_horizontal_fence_tile")
	IMG("pak::bin\\graphics\\objects\\decal\\fences\\vertical_fence_tile.png",                "Data/Atlases/Decals.xml",                        "decal_fence_vertical_fence_tile")
	IMG("pak::bin\\graphics\\objects\\decal\\furniture\\bed.png",                             "Data/Atlases/Decals.xml",                        "decal_furniture_bed")
	IMG("pak::bin\\graphics\\objects\\decal\\furniture\\fridge.png",                          "Data/Atlases/Decals.xml",                        "decal_furniture_fridge")
	IMG("pak::bin\\graphics\\objects\\decal\\furniture\\hob.png",                             "Data/Atlases/Decals.xml",                        "decal_furniture_hob")
	IMG("pak::bin\\graphics\\objects\\decal\\inside\\toilet.png",                             "Data/Atlases/Decals.xml",                        "decal_inside_toilet")
	IMG("pak::bin\\graphics\\objects\\decal\\lab\\air_vent.png",                              "Data/Atlases/Decals.xml",                        "decal_lab_air_vent")
	IMG("pak::bin\\graphics\\objects\\decal\\lab\\gas_silo.png",                              "Data/Atlases/Decals.xml",                        "decal_lab_gas_silo")
	IMG("pak::bin\\graphics\\objects\\decal\\lab\\table_horizontal.png",                      "Data/Atlases/Decals.xml",                        "decal_lab_table_horizontal")
	IMG("pak::bin\\graphics\\objects\\decal\\lab\\table_vertical.png",                        "Data/Atlases/Decals.xml",                        "decal_lab_table_vertical")
	IMG("pak::bin\\graphics\\objects\\decal\\lab\\wall_vent.png",                             "Data/Atlases/Decals.xml",                        "decal_lab_wall_vent")
	IMG("pak::bin\\graphics\\objects\\decal\\lab\\zombie_jar.png",                            "Data/Atlases/Decals.xml",                        "decal_lab_zombie_jar")
	IMG("pak::bin\\graphics\\objects\\decal\\lights\\light_1.png",                            "Data/Atlases/Decals.xml",                        "decal_light_light_1")
	IMG("pak::bin\\graphics\\objects\\decal\\lights\\tall_light.png",                         "Data/Atlases/Decals.xml",                        "decal_light_tall_light")
	IMG("pak::bin\\graphics\\objects\\decal\\lights\\wall_lamp.png",                          "Data/Atlases/Decals.xml",                        "decal_light_wall_lamp")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\barricade_horizontal.png",                 "Data/Atlases/Decals.xml",                        "decal_misc_barricade_horizontal")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\bench_down.png",                           "Data/Atlases/Decals.xml",                        "decal_misc_bench_down")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\bench_left.png",                           "Data/Atlases/Decals.xml",                        "decal_misc_bench_left")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\bench_right.png",                          "Data/Atlases/Decals.xml",                        "decal_misc_bench_right")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\bricks_rgb.png",                           "Data/Atlases/Decals.xml",                        "decal_misc_bricks_rgb")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\bricks.ini",		                          "Data/Atlases/Decals.xml",                        "decal_misc_bricks_rgb")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\centerpiece.png",                          "Data/Atlases/Decals.xml",                        "decal_misc_centerpiece")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\cone.png",                                 "Data/Atlases/Decals.xml",                        "decal_misc_cone")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\gravestone_rgb.png",                       "Data/Atlases/Decals.xml",                        "decal_misc_gravestone_rgb")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\gravestone.ini",                           "Data/Atlases/Decals.xml",                        "decal_misc_gravestone_rgb")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\hydrant_rgb.png",                          "Data/Atlases/Decals.xml",                        "decal_misc_hydrant_rgb")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\hydrant.ini",                              "Data/Atlases/Decals.xml",                        "decal_misc_hydrant_rgb")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\logs.png",                                 "Data/Atlases/Decals.xml",                        "decal_misc_logs")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\pole_diagonal_ns.png",                     "Data/Atlases/Decals.xml",                        "decal_misc_pole_diagonal_ns")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\pole_diagonal_sn.png",                     "Data/Atlases/Decals.xml",                        "decal_misc_pole_diagonal_sn")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\pole_horizontal.png",                      "Data/Atlases/Decals.xml",                        "decal_misc_pole_horizontal")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\pole_single.png",                          "Data/Atlases/Decals.xml",                        "decal_misc_pole_single")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\pole_vertical.png",                        "Data/Atlases/Decals.xml",                        "decal_misc_pole_vertical")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\rocks.png",                                "Data/Atlases/Decals.xml",                        "decal_misc_rocks")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\stencil_numbers.png",                      "Data/Atlases/Decals.xml",                        "decal_misc_stencil_numbers")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\numbers.ini",		                      "Data/Atlases/Decals.xml",                        "decal_misc_stencil_numbers")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\tree_log.png",                             "Data/Atlases/Decals.xml",                        "decal_misc_tree_log")
	IMG("pak::bin\\graphics\\objects\\decal\\misc\\vault.png",                                "Data/Atlases/Decals.xml",                        "decal_misc_vault")
	IMG("pak::bin\\graphics\\objects\\decal\\paths\\sand_1.png",                              "Data/Atlases/Decals.xml",                        "decal_path_sand_1")
	IMG("pak::bin\\graphics\\objects\\decal\\paths\\urban.png",                               "Data/Atlases/Decals.xml",                        "decal_path_urban")
	IMG("pak::bin\\graphics\\objects\\decal\\posters\\poster_alert.png",                      "Data/Atlases/Decals.xml",                        "decal_poster_poster_alert")
	IMG("pak::bin\\graphics\\objects\\decal\\posters\\poster_biffs_burgers.png",              "Data/Atlases/Decals.xml",                        "decal_poster_poster_biffs_burgers")
	IMG("pak::bin\\graphics\\objects\\decal\\posters\\poster_claymores.png",                  "Data/Atlases/Decals.xml",                        "decal_poster_poster_claymores")
	IMG("pak::bin\\graphics\\objects\\decal\\posters\\poster_escape.png",                     "Data/Atlases/Decals.xml",                        "decal_poster_poster_escape")
	IMG("pak::bin\\graphics\\objects\\decal\\posters\\poster_magnum.png",                     "Data/Atlases/Decals.xml",                        "decal_poster_poster_magnum")
	IMG("pak::bin\\graphics\\objects\\decal\\posters\\poster_narrow.png",                     "Data/Atlases/Decals.xml",                        "decal_poster_poster_narrow")
	IMG("pak::bin\\graphics\\objects\\decal\\posters\\poster_narrow_1.png",                   "Data/Atlases/Decals.xml",                        "decal_poster_poster_narrow_1")
	IMG("pak::bin\\graphics\\objects\\decal\\posters\\poster_paint.png",                      "Data/Atlases/Decals.xml",                        "decal_poster_poster_paint")
	IMG("pak::bin\\graphics\\objects\\decal\\posters\\poster_wanted.png",                     "Data/Atlases/Decals.xml",                        "decal_poster_poster_wanted")
	IMG("pak::bin\\graphics\\objects\\decal\\posters\\poster_wide.png",                       "Data/Atlases/Decals.xml",                        "decal_poster_poster_wide")
	IMG("pak::bin\\graphics\\objects\\decal\\posters\\poster_wide_1.png",                     "Data/Atlases/Decals.xml",                        "decal_poster_poster_wide_1")
	IMG("pak::bin\\graphics\\objects\\decal\\rack\\rack_1.png",                               "Data/Atlases/Decals.xml",                        "decal_rack_rack_1")
	IMG("pak::bin\\graphics\\objects\\decal\\rack\\rack_2.png",                               "Data/Atlases/Decals.xml",                        "decal_rack_rack_2")
	IMG("pak::bin\\graphics\\objects\\decal\\rack\\rack_3.png",                               "Data/Atlases/Decals.xml",                        "decal_rack_rack_3")
	IMG("pak::bin\\graphics\\objects\\decal\\rubbish\\ball_1.png",                            "Data/Atlases/Decals.xml",                        "decal_rubbish_ball_1")
	IMG("pak::bin\\graphics\\objects\\decal\\rubbish\\bottle_side_brown.png",                 "Data/Atlases/Decals.xml",                        "decal_rubbish_bottle_side_brown")
	IMG("pak::bin\\graphics\\objects\\decal\\rubbish\\bottle_side_green.png",                 "Data/Atlases/Decals.xml",                        "decal_rubbish_bottle_side_green")
	IMG("pak::bin\\graphics\\objects\\decal\\rubbish\\bottle_upright_brown.png",              "Data/Atlases/Decals.xml",                        "decal_rubbish_bottle_upright_brown")
	IMG("pak::bin\\graphics\\objects\\decal\\rubbish\\bottle_upright_green.png",              "Data/Atlases/Decals.xml",                        "decal_rubbish_bottle_upright_green")
	IMG("pak::bin\\graphics\\objects\\decal\\rubbish\\newspaper_1.png",                       "Data/Atlases/Decals.xml",                        "decal_rubbish_newspaper_1")
	IMG("pak::bin\\graphics\\objects\\decal\\rubbish\\newspaper_2.png",                       "Data/Atlases/Decals.xml",                        "decal_rubbish_newspaper_2")
	IMG("pak::bin\\graphics\\objects\\decal\\shopping\\carts.png",                            "Data/Atlases/Decals.xml",                        "decal_shopping_carts")
	IMG("pak::bin\\graphics\\objects\\decal\\signs\\24_7_large.png",                          "Data/Atlases/Decals.xml",                        "decal_sign_24_7_large")
	IMG("pak::bin\\graphics\\objects\\decal\\signs\\24_7_small.png",                          "Data/Atlases/Decals.xml",                        "decal_sign_24_7_small")
	IMG("pak::bin\\graphics\\objects\\decal\\signs\\advert.png",                              "Data/Atlases/Decals.xml",                        "decal_sign_advert")
	IMG("pak::bin\\graphics\\objects\\decal\\signs\\bank.png",                                "Data/Atlases/Decals.xml",                        "decal_sign_bank")
	IMG("pak::bin\\graphics\\objects\\decal\\signs\\buffco.png",                              "Data/Atlases/Decals.xml",                        "decal_sign_buffco")
	IMG("pak::bin\\graphics\\objects\\decal\\signs\\buff_beacon.png",                         "Data/Atlases/Decals.xml",                        "decal_sign_buff_beacon")
	IMG("pak::bin\\graphics\\objects\\decal\\signs\\game_center.png",                         "Data/Atlases/Decals.xml",                        "decal_sign_game_center")
	IMG("pak::bin\\graphics\\objects\\decal\\signs\\hospital.png",                            "Data/Atlases/Decals.xml",                        "decal_sign_hospital")
	IMG("pak::bin\\graphics\\objects\\decal\\signs\\pole.png",                                "Data/Atlases/Decals.xml",                        "decal_sign_pole")
	IMG("pak::bin\\graphics\\objects\\decal\\signs\\pole_small.png",                          "Data/Atlases/Decals.xml",                        "decal_sign_pole_small")
	IMG("pak::bin\\graphics\\objects\\decal\\signs\\salon.png",                               "Data/Atlases/Decals.xml",                        "decal_sign_salon")
	IMG("pak::bin\\graphics\\objects\\decal\\signs\\small.png",                               "Data/Atlases/Decals.xml",                        "decal_sign_small")
	IMG("pak::bin\\graphics\\objects\\decal\\signs\\stat_display.png",                        "Data/Atlases/Decals.xml",                        "decal_sign_stat_display")
	IMG("pak::bin\\graphics\\objects\\decal\\signs\\street_lights.png",                       "Data/Atlases/Decals.xml",                        "decal_sign_street_lights")
	IMG("pak::bin\\graphics\\objects\\decal\\signs\\street_sign.png",                         "Data/Atlases/Decals.xml",                        "decal_sign_street_sign")
	IMG("pak::bin\\graphics\\objects\\decal\\signs\\subway.png",                              "Data/Atlases/Decals.xml",                        "decal_sign_subway")
	IMG("pak::bin\\graphics\\objects\\decal\\signs\\welcome.png",                             "Data/Atlases/Decals.xml",                        "decal_sign_welcome")
	IMG("pak::bin\\graphics\\objects\\decal\\vehicles\\truck_buffco.png",                     "Data/Atlases/Decals.xml",                        "decal_vehicle_truck_buffco")
	IMG("pak::bin\\graphics\\objects\\decal\\vehicles\\truck_drsoda.png",                     "Data/Atlases/Decals.xml",                        "decal_vehicle_truck_drsoda")
	IMG("pak::bin\\graphics\\objects\\decal\\winter_map\\background.png",                     "Data/Atlases/Decals.xml",                        "decal_winter_background")
	IMG("pak::bin\\graphics\\objects\\decal\\winter_map\\barricade1.png",                     "Data/Atlases/Decals.xml",                        "decal_winter_barricade1")
	IMG("pak::bin\\graphics\\objects\\decal\\winter_map\\barricade2.png",                     "Data/Atlases/Decals.xml",                        "decal_winter_barricade2")
	IMG("pak::bin\\graphics\\objects\\decal\\winter_map\\barricade3.png",                     "Data/Atlases/Decals.xml",                        "decal_winter_barricade3")
	IMG("pak::bin\\graphics\\objects\\decal\\winter_map\\barricade4.png",                     "Data/Atlases/Decals.xml",                        "decal_winter_barricade4")
	IMG("pak::bin\\graphics\\objects\\decal\\winter_map\\dirt1.png",                          "Data/Atlases/Decals.xml",                        "decal_winter_dirt1")
	IMG("pak::bin\\graphics\\objects\\decal\\winter_map\\dirt2.png",                          "Data/Atlases/Decals.xml",                        "decal_winter_dirt2")
	IMG("pak::bin\\graphics\\objects\\decal\\winter_map\\dirt3.png",                          "Data/Atlases/Decals.xml",                        "decal_winter_dirt3")
	IMG("pak::bin\\graphics\\objects\\decal\\winter_map\\dirt4.png",                          "Data/Atlases/Decals.xml",                        "decal_winter_dirt4")
	IMG("pak::bin\\graphics\\objects\\decal\\winter_map\\fire_pit.png",                       "Data/Atlases/Decals.xml",                        "decal_winter_fire_pit")
	IMG("pak::bin\\graphics\\objects\\decal\\winter_map\\snow_bank.png",                      "Data/Atlases/Decals.xml",                        "decal_winter_snow_bank")
	IMG("pak::bin\\graphics\\objects\\decal\\winter_map\\snow_corpses.png",                   "Data/Atlases/Decals.xml",                        "decal_winter_snow_corpses")
	IMG("pak::bin\\graphics\\objects\\decal\\winter_map\\snow_pine.png",                      "Data/Atlases/Decals.xml",                        "decal_winter_snow_pine")
	IMG("pak::bin\\graphics\\objects\\decal\\winter_map\\snow_rocks.png",                     "Data/Atlases/Decals.xml",                        "decal_winter_snow_rocks")
	IMG("pak::bin\\graphics\\objects\\decal\\winter_map\\snow_tree.png",                      "Data/Atlases/Decals.xml",                        "decal_winter_snow_tree")
	IMG("pak::bin\\graphics\\objects\\destroyable\\default.png",                              "Data/Atlases/Actors.xml",                        "actor_destroyable_default")
	IMG("pak::bin\\graphics\\objects\\destroyable\\explosion.png",                            "Data/Atlases/Actors.xml",                        "actor_destroyable_explosion")
	IMG("pak::bin\\graphics\\objects\\destroyable\\explosion_remains.png",                    "Data/Atlases/Actors.xml",                        "actor_destroyable_remains")
	IMG("pak::bin\\graphics\\objects\\destroyable\\gibs.png",                                 "Data/Atlases/Actors.xml",                        "actor_destroyable_gibs")
	IMG("pak::bin\\graphics\\objects\\destroyable\\smoke.png",                                "Data/Atlases/Actors.xml",                        "actor_destroyable_smoke")
	IMG("pak::bin\\graphics\\objects\\doors\\default.png",                                    "Data/Atlases/Actors.xml",                        "actor_door_default")
	IMG("pak::bin\\graphics\\objects\\doors\\door_1.png",                                     "Data/Atlases/Actors.xml",                        "actor_door_door_1")
	IMG("pak::bin\\graphics\\objects\\doors\\door_2.png",                                     "Data/Atlases/Actors.xml",                        "actor_door_door_2")
	IMG("pak::bin\\graphics\\objects\\doors\\exit_door.png",                                  "Data/Atlases/Actors.xml",                        "actor_door_exit_door")
	IMG("pak::bin\\graphics\\objects\\doors\\exit_door_debris.png",                           "Data/Atlases/Actors.xml",                        "actor_door_exit_door_debris")
	IMG("pak::bin\\graphics\\objects\\doors\\exit_sign.png",                                  "Data/Atlases/Actors.xml",                        "actor_door_exit_sign")
	IMG("pak::bin\\graphics\\objects\\doors\\explosion_large.png",                            "Data/Atlases/Actors.xml",                        "actor_door_explosion_large")
	IMG("pak::bin\\graphics\\objects\\doors\\explosive.png",                                  "Data/Atlases/Actors.xml",                        "actor_door_explosive")
	IMG("pak::bin\\graphics\\objects\\doors\\inside_door.png",                                "Data/Atlases/Actors.xml",                        "actor_door_inside_door")
	IMG("pak::bin\\graphics\\objects\\doors\\laser_blueh.png",                                "Data/Atlases/Actors.xml",                        "actor_door_laser_blueh")
	IMG("pak::bin\\graphics\\objects\\doors\\laser_bluev.png",                                "Data/Atlases/Actors.xml",                        "actor_door_laser_bluev")
	IMG("pak::bin\\graphics\\objects\\doors\\laser_greenh.png",                               "Data/Atlases/Actors.xml",                        "actor_door_laser_greenh")
	IMG("pak::bin\\graphics\\objects\\doors\\laser_greenv.png",                               "Data/Atlases/Actors.xml",                        "actor_door_laser_greenv")
	IMG("pak::bin\\graphics\\objects\\doors\\laser_redh.png",                                 "Data/Atlases/Actors.xml",                        "actor_door_laser_redh")
	IMG("pak::bin\\graphics\\objects\\doors\\laser_redv.png",                                 "Data/Atlases/Actors.xml",                        "actor_door_laser_redv")
	IMG("pak::bin\\graphics\\objects\\doors\\laser_yellowh.png",                              "Data/Atlases/Actors.xml",                        "actor_door_laser_yellowh")
	IMG("pak::bin\\graphics\\objects\\doors\\laser_yellowv.png",                              "Data/Atlases/Actors.xml",                        "actor_door_laser_yellowv")
	IMG("pak::bin\\graphics\\objects\\doors\\pip.png",                                        "Data/Atlases/Actors.xml",                        "actor_door_pip")
	IMG("pak::bin\\graphics\\objects\\doors\\toilet.png",                                     "Data/Atlases/Actors.xml",                        "actor_door_toilet")
	IMG("pak::bin\\graphics\\objects\\doors\\colour coded\\blue.png",                         "Data/Atlases/Actors.xml",                        "actor_door_blue")
	IMG("pak::bin\\graphics\\objects\\doors\\colour coded\\green.png",                        "Data/Atlases/Actors.xml",                        "actor_door_green")
	IMG("pak::bin\\graphics\\objects\\doors\\colour coded\\red.png",                          "Data/Atlases/Actors.xml",                        "actor_door_red")
	IMG("pak::bin\\graphics\\objects\\doors\\colour coded\\yellow.png",                       "Data/Atlases/Actors.xml",                        "actor_door_yellow")
	IMG("pak::bin\\graphics\\objects\\emitter\\default.png",                                  "Data/Atlases/Actors.xml",                        "actor_emitter_default")
	IMG("pak::bin\\graphics\\objects\\enemies\\boss1\\gibs.png",                              "Data/Atlases/NPCs.xml",                          "enemy_boss1_gibs")
	IMG("pak::bin\\graphics\\objects\\enemies\\boss1\\zombie.png",                            "Data/Atlases/NPCs.xml",                          "enemy_boss1_zombie")
	IMG("pak::bin\\graphics\\objects\\enemies\\bouncer\\gibs.png",                            "Data/Atlases/NPCs.xml",                          "enemy_bouncer_gibs")
	IMG("pak::bin\\graphics\\objects\\enemies\\bouncer\\zombie.png",                          "Data/Atlases/NPCs.xml",                          "enemy_bouncer_zombie")
	IMG("pak::bin\\graphics\\objects\\enemies\\exploder\\gibs.png",                           "Data/Atlases/NPCs.xml",                          "enemy_exploder_gibs")
	IMG("pak::bin\\graphics\\objects\\enemies\\exploder\\zombie.png",                         "Data/Atlases/NPCs.xml",                          "enemy_exploder_zombie")
	IMG("pak::bin\\graphics\\objects\\enemies\\fodder\\gibs.png",                             "Data/Atlases/NPCs.xml",                          "enemy_fodder_gibs")
	IMG("pak::bin\\graphics\\objects\\enemies\\fodder\\zombie.png",                           "Data/Atlases/NPCs.xml",                          "enemy_fodder_zombie")
	IMG("pak::bin\\graphics\\objects\\enemies\\zombie\\fodder\\gibs.png",                     "Data/Atlases/NPCs.xml",                          "enemy_fodder_gibs")
	IMG("pak::bin\\graphics\\objects\\enemies\\zombie\\fodder\\zombie.png",                   "Data/Atlases/NPCs.xml",                          "enemy_fodder_zombie")
	IMG("pak::bin\\graphics\\objects\\enemies\\trailer\\gibs.png",                            "Data/Atlases/NPCs.xml",                          "enemy_trailer_gibs")
	IMG("pak::bin\\graphics\\objects\\enemies\\trailer\\zombie.png",                          "Data/Atlases/NPCs.xml",                          "enemy_trailer_zombie")
	IMG("pak::bin\\graphics\\objects\\enemies\\zombie\\blood.png",                            "Data/Atlases/NPCs.xml",                          "enemy_zombie_blood")
	IMG("pak::bin\\graphics\\objects\\enemies\\zombie\\gibs.png",                             "Data/Atlases/NPCs.xml",                          "enemy_zombie_gibs")
	IMG("pak::bin\\graphics\\objects\\enemies\\zombie\\splat.png",                            "Data/Atlases/NPCs.xml",                          "enemy_zombie_splat")
	IMG("pak::bin\\graphics\\objects\\enemies\\zombie\\zombie.png",                           "Data/Atlases/NPCs.xml",                          "enemy_zombie_zombie")
	IMG("pak::bin\\graphics\\objects\\light\\blend.png",                                      "Data/Atlases/Actors.xml",                        "actor_light_blend")
	IMG("pak::bin\\graphics\\objects\\light\\default.png",                                    "Data/Atlases/Actors.xml",                        "actor_light_default")
	IMG("pak::bin\\graphics\\objects\\light\\tile_light.png",                                 "Data/Atlases/Actors.xml",                        "actor_light_light")
	IMG("pak::bin\\graphics\\objects\\locks\\default.png",                                    "Data/Atlases/Actors.xml",                        "actor_lock_default")
	IMG("pak::bin\\graphics\\objects\\map\\c1_s1\\pedobear.png",                              "Data/Atlases/Actors.xml",                        "actor_map_c1_s1_pedobear")
	IMG("pak::bin\\graphics\\objects\\map\\c1_s1\\pedobear_overlay.png",                      "Data/Atlases/Actors.xml",                        "actor_map_c1_s1_overlay")
	IMG("pak::bin\\graphics\\objects\\map\\tutorial\\nothingtoseehere.png",                   "Data/Atlases/Actors.xml",                        "actor_map_tutorial_nothingtoseehere")
	IMG("pak::bin\\graphics\\objects\\map\\tutorial\\takeit.png",                             "Data/Atlases/Actors.xml",                        "actor_map_tutorial_takeit")
	IMG("pak::bin\\graphics\\objects\\map\\tutorial\\title_sign.png",                         "Data/Atlases/Actors.xml",                        "actor_map_tutorial_sign")
	IMG("pak::bin\\graphics\\objects\\marker\\default.png",                                   "Data/Atlases/Actors.xml",                        "actor_marker_default")
	IMG("pak::bin\\graphics\\objects\\move zone\\default.png",                                "Data/Atlases/Actors.xml",                        "actor_move_zone_default")
	IMG("pak::bin\\graphics\\objects\\npc\\nurse\\nurse.png",                                 "Data/Atlases/NPCs.xml",                          "npc_nurse_nurse")
	IMG("pak::bin\\graphics\\objects\\npc\\stylist\\stylist.png",                             "Data/Atlases/NPCs.xml",                          "npc_stylist_stylist")
	IMG("pak::bin\\graphics\\objects\\npc\\survivor\\arrow_pip.png",                          "Data/Atlases/NPCs.xml",                          "npc_survivor_arrow_pip")
	IMG("pak::bin\\graphics\\objects\\npc\\survivor\\blood.png",                              "Data/Atlases/NPCs.xml",                          "npc_survivor_blood")
	IMG("pak::bin\\graphics\\objects\\npc\\survivor\\gibs.png",                               "Data/Atlases/NPCs.xml",                          "npc_survivor_gibs")
	IMG("pak::bin\\graphics\\objects\\npc\\survivor\\puff.png",                               "Data/Atlases/NPCs.xml",                          "npc_survivor_puff")
	IMG("pak::bin\\graphics\\objects\\npc\\survivor\\splat.png",                              "Data/Atlases/NPCs.xml",                          "npc_survivor_splat")
	IMG("pak::bin\\graphics\\objects\\npc\\survivor\\survivor.png",                           "Data/Atlases/NPCs.xml",                          "npc_survivor_survivor")
	IMG("pak::bin\\graphics\\objects\\npc\\survivor\\survivor_zone.png",                      "Data/Atlases/NPCs.xml",                          "npc_survivor_survivor_zone")
	IMG("pak::bin\\graphics\\objects\\pickups\\acid_gun.png",                                 "Data/Atlases/Actors.xml",                        "actor_pickups_acid_gun")
	IMG("pak::bin\\graphics\\objects\\pickups\\ak47.png",                                     "Data/Atlases/Actors.xml",                        "actor_pickups_ak47")
	IMG("pak::bin\\graphics\\objects\\pickups\\ammo.png",                                     "Data/Atlases/Actors.xml",                        "actor_pickups_ammo")
	IMG("pak::bin\\graphics\\objects\\pickups\\armour_large.png",                             "Data/Atlases/Actors.xml",                        "actor_pickups_armour_large")
	IMG("pak::bin\\graphics\\objects\\pickups\\armour_medium.png",                            "Data/Atlases/Actors.xml",                        "actor_pickups_armour_medium")
	IMG("pak::bin\\graphics\\objects\\pickups\\armour_small.png",                             "Data/Atlases/Actors.xml",                        "actor_pickups_armour_small")
	IMG("pak::bin\\graphics\\objects\\pickups\\bag.png",                                      "Data/Atlases/Actors.xml",                        "actor_pickups_bag")
	IMG("pak::bin\\graphics\\objects\\pickups\\bag_no_outline.png",                           "Data/Atlases/Actors.xml",                        "actor_pickups_bag_no_outline")
	IMG("pak::bin\\graphics\\objects\\pickups\\baseball_bat.png",                             "Data/Atlases/Actors.xml",                        "actor_pickups_baseball_bat")
	IMG("pak::bin\\graphics\\objects\\pickups\\bolt_gun.png",                                 "Data/Atlases/Actors.xml",                        "actor_pickups_bolt_gun")
	IMG("pak::bin\\graphics\\objects\\pickups\\buff_grenade_ammo.png",                        "Data/Atlases/Actors.xml",                        "actor_pickups_buff_grenade_ammo")
	IMG("pak::bin\\graphics\\objects\\pickups\\buff_grenade_damage.png",                      "Data/Atlases/Actors.xml",                        "actor_pickups_buff_grenade_damage")
	IMG("pak::bin\\graphics\\objects\\pickups\\buff_grenade_health.png",                      "Data/Atlases/Actors.xml",                        "actor_pickups_buff_grenade_health")
	IMG("pak::bin\\graphics\\objects\\pickups\\buff_grenade_price.png",                       "Data/Atlases/Actors.xml",                        "actor_pickups_buff_grenade_price")
	IMG("pak::bin\\graphics\\objects\\pickups\\buff_grenade_reload.png",                      "Data/Atlases/Actors.xml",                        "actor_pickups_buff_grenade_reload")
	IMG("pak::bin\\graphics\\objects\\pickups\\buff_grenade_rof.png",                         "Data/Atlases/Actors.xml",                        "actor_pickups_buff_grenade_rof")
	IMG("pak::bin\\graphics\\objects\\pickups\\buff_grenade_speed.png",                       "Data/Atlases/Actors.xml",                        "actor_pickups_buff_grenade_speed")
	IMG("pak::bin\\graphics\\objects\\pickups\\buff_grenade_xp.png",                          "Data/Atlases/Actors.xml",                        "actor_pickups_buff_grenade_xp")
	IMG("pak::bin\\graphics\\objects\\pickups\\cardkeys.png",                                 "Data/Atlases/Actors.xml",                        "actor_pickups_cardkeys")
	IMG("pak::bin\\graphics\\objects\\pickups\\chest_key.png",                                "Data/Atlases/Actors.xml",                        "actor_pickups_chest_key")
	IMG("pak::bin\\graphics\\objects\\pickups\\coin.png",                                     "Data/Atlases/Actors.xml",                        "actor_pickups_coin")
	IMG("pak::bin\\graphics\\objects\\pickups\\doublebarrelshotgun.png",                      "Data/Atlases/Actors.xml",                        "actor_pickups_doublebarrelshotgun")
	IMG("pak::bin\\graphics\\objects\\pickups\\drink.png",                                    "Data/Atlases/Actors.xml",                        "actor_pickups_drink")
	IMG("pak::bin\\graphics\\objects\\pickups\\explosiveammo.png",                            "Data/Atlases/Actors.xml",                        "actor_pickups_explosiveammo")
	IMG("pak::bin\\graphics\\objects\\pickups\\fight_saber.png",                              "Data/Atlases/Actors.xml",                        "actor_pickups_fight_saber")
	IMG("pak::bin\\graphics\\objects\\pickups\\fire_turret.png",                              "Data/Atlases/Actors.xml",                        "actor_pickups_fire_turret")
	IMG("pak::bin\\graphics\\objects\\pickups\\flamethrower.png",                             "Data/Atlases/Actors.xml",                        "actor_pickups_flamethrower")
	IMG("pak::bin\\graphics\\objects\\pickups\\flaregun.png",                                 "Data/Atlases/Actors.xml",                        "actor_pickups_flaregun")
	IMG("pak::bin\\graphics\\objects\\pickups\\freezeammo.png",                               "Data/Atlases/Actors.xml",                        "actor_pickups_freezeammo")
	IMG("pak::bin\\graphics\\objects\\pickups\\freeze_ray.png",                               "Data/Atlases/Actors.xml",                        "actor_pickups_freeze_ray")
	IMG("pak::bin\\graphics\\objects\\pickups\\grenade_launcher.png",                         "Data/Atlases/Actors.xml",                        "actor_pickups_grenade_launcher")
	IMG("pak::bin\\graphics\\objects\\pickups\\incendiaryammo.png",                           "Data/Atlases/Actors.xml",                        "actor_pickups_incendiaryammo")
	IMG("pak::bin\\graphics\\objects\\pickups\\key.png",                                      "Data/Atlases/Actors.xml",                        "actor_pickups_key")
	IMG("pak::bin\\graphics\\objects\\pickups\\lasercannon.png",                              "Data/Atlases/Actors.xml",                        "actor_pickups_lasercannon")
	IMG("pak::bin\\graphics\\objects\\pickups\\lovecannon.png",                               "Data/Atlases/Actors.xml",                        "actor_pickups_lovecannon")
	IMG("pak::bin\\graphics\\objects\\pickups\\lovethrower.png",                              "Data/Atlases/Actors.xml",                        "actor_pickups_lovethrower")
	IMG("pak::bin\\graphics\\objects\\pickups\\m16.png",                                      "Data/Atlases/Actors.xml",                        "actor_pickups_m16")
	IMG("pak::bin\\graphics\\objects\\pickups\\magnum.png",                                   "Data/Atlases/Actors.xml",                        "actor_pickups_magnum")
	IMG("pak::bin\\graphics\\objects\\pickups\\map.png",                                      "Data/Atlases/Actors.xml",                        "actor_pickups_map")
	IMG("pak::bin\\graphics\\objects\\pickups\\medipack.png",                                 "Data/Atlases/Actors.xml",                        "actor_pickups_medipack")
	IMG("pak::bin\\graphics\\objects\\pickups\\nachos.png",                                   "Data/Atlases/Actors.xml",                        "actor_pickups_nachos")
	IMG("pak::bin\\graphics\\objects\\pickups\\pill.png",                                     "Data/Atlases/Actors.xml",                        "actor_pickups_pill")
	IMG("pak::bin\\graphics\\objects\\pickups\\pillcontainer.png",                            "Data/Atlases/Actors.xml",                        "actor_pickups_pillcontainer")
	IMG("pak::bin\\graphics\\objects\\pickups\\random.png",                                   "Data/Atlases/Actors.xml",                        "actor_pickups_random")
	IMG("pak::bin\\graphics\\objects\\pickups\\rocket_launcher.png",                          "Data/Atlases/Actors.xml",                        "actor_pickups_rocket_launcher")
	IMG("pak::bin\\graphics\\objects\\pickups\\rocket_turret.png",                            "Data/Atlases/Actors.xml",                        "actor_pickups_rocket_turret")
	IMG("pak::bin\\graphics\\objects\\pickups\\rpc.png",                                      "Data/Atlases/Actors.xml",                        "actor_pickups_rpc")
	IMG("pak::bin\\graphics\\objects\\pickups\\scatter_gun.png",                              "Data/Atlases/Actors.xml",                        "actor_pickups_scatter_gun")
	IMG("pak::bin\\graphics\\objects\\pickups\\shock_rifle.png",                              "Data/Atlases/Actors.xml",                        "actor_pickups_shock_rifle")
	IMG("pak::bin\\graphics\\objects\\pickups\\shotgun.png",                                  "Data/Atlases/Actors.xml",                        "actor_pickups_shotgun")
	IMG("pak::bin\\graphics\\objects\\pickups\\spaz.png",                                     "Data/Atlases/Actors.xml",                        "actor_pickups_spaz")
	IMG("pak::bin\\graphics\\objects\\pickups\\sticky_launcher.png",                          "Data/Atlases/Actors.xml",                        "actor_pickups_sticky_launcher")
	IMG("pak::bin\\graphics\\objects\\pickups\\tripmine.png",                                 "Data/Atlases/Actors.xml",                        "actor_pickups_tripmine")
	IMG("pak::bin\\graphics\\objects\\pickups\\turret.png",                                   "Data/Atlases/Actors.xml",                        "actor_pickups_turret")
	IMG("pak::bin\\graphics\\objects\\pickups\\uzi.png",                                      "Data/Atlases/Actors.xml",                        "actor_pickups_uzi")
	IMG("pak::bin\\graphics\\objects\\pickups\\winchester_rifle.png",                         "Data/Atlases/Actors.xml",                        "actor_pickups_winchester_rifle")
	IMG("pak::bin\\graphics\\objects\\player start\\default.png",                             "Data/Atlases/Actors.xml",                        "actor_player_start_default")
	IMG("pak::bin\\graphics\\objects\\projectile spawner\\default.png",                       "Data/Atlases/Actors.xml",                        "actor_projectile_spawner_default")
	IMG("pak::bin\\graphics\\objects\\reflection zone\\default.png",                          "Data/Atlases/Actors.xml",                        "actor_reflection_zone_default")
	IMG("pak::bin\\graphics\\objects\\shaders\\default.png",                                  "Data/Atlases/Actors.xml",                        "actor_shaders_default")
	IMG("pak::bin\\graphics\\objects\\sound source\\default.png",                             "Data/Atlases/Actors.xml",                        "actor_sound_source_default")
	IMG("pak::bin\\graphics\\objects\\spawn chamber\\bottom.png",                             "Data/Atlases/Actors.xml",                        "actor_spawn_chamber_bottom")
	IMG("pak::bin\\graphics\\objects\\spawn chamber\\puff.png",                               "Data/Atlases/Actors.xml",                        "actor_spawn_chamber_puff")
	IMG("pak::bin\\graphics\\objects\\spawn chamber\\sign.png",                               "Data/Atlases/Actors.xml",                        "actor_spawn_chamber_sign")
	IMG("pak::bin\\graphics\\objects\\spawn chamber\\tele_phase.png",                         "Data/Atlases/Actors.xml",                        "actor_spawn_chamber_tele_phase")
	IMG("pak::bin\\graphics\\objects\\spawn chamber\\top.png",                                "Data/Atlases/Actors.xml",                        "actor_spawn_chamber_top")
	IMG("pak::bin\\graphics\\objects\\switchs\\blue.png",                                     "Data/Atlases/Actors.xml",                        "actor_switch_blue")
	IMG("pak::bin\\graphics\\objects\\switchs\\default.png",                                  "Data/Atlases/Actors.xml",                        "actor_switch_default")
	IMG("pak::bin\\graphics\\objects\\switchs\\green.png",                                    "Data/Atlases/Actors.xml",                        "actor_switch_green")
	IMG("pak::bin\\graphics\\objects\\switchs\\pressure_plate.png",                           "Data/Atlases/Actors.xml",                        "actor_switch_pressure_plate")
	IMG("pak::bin\\graphics\\objects\\switchs\\pressure_plate_blue.png",                      "Data/Atlases/Actors.xml",                        "actor_switch_pressure_plate_blue")
	IMG("pak::bin\\graphics\\objects\\switchs\\pressure_plate_green.png",                     "Data/Atlases/Actors.xml",                        "actor_switch_pressure_plate_green")
	IMG("pak::bin\\graphics\\objects\\switchs\\pressure_plate_red.png",                       "Data/Atlases/Actors.xml",                        "actor_switch_pressure_plate_red")
	IMG("pak::bin\\graphics\\objects\\switchs\\pressure_plate_yellow.png",                    "Data/Atlases/Actors.xml",                        "actor_switch_pressure_plate_yellow")
	IMG("pak::bin\\graphics\\objects\\switchs\\yellow.png",                                   "Data/Atlases/Actors.xml",                        "actor_switch_yellow")
	IMG("pak::bin\\graphics\\objects\\trigger\\default.png",                                  "Data/Atlases/Actors.xml",                        "actor_trigger_default")
	IMG("pak::bin\\graphics\\objects\\trigger\\map_change.png",                               "Data/Atlases/Actors.xml",                        "actor_trigger_change")
	IMG("pak::bin\\graphics\\objects\\trigger\\map_exit.png",                                 "Data/Atlases/Actors.xml",                        "actor_trigger_exit")
	IMG("pak::bin\\graphics\\objects\\turrets\\ammo_bubble.png",                              "Data/Atlases/Actors.xml",                        "actor_turrets_ammo_bubble")
	IMG("pak::bin\\graphics\\objects\\turrets\\base.png",                                     "Data/Atlases/Actors.xml",                        "actor_turrets_base")
	IMG("pak::bin\\graphics\\objects\\turrets\\fire_turret.png",                              "Data/Atlases/Actors.xml",                        "actor_turrets_fire_turret")
	IMG("pak::bin\\graphics\\objects\\turrets\\gibs.png",                                     "Data/Atlases/Actors.xml",                        "actor_turrets_gibs")
	IMG("pak::bin\\graphics\\objects\\turrets\\puff.png",                                     "Data/Atlases/Actors.xml",                        "actor_turrets_puff")
	IMG("pak::bin\\graphics\\objects\\turrets\\rocket_turret.png",                            "Data/Atlases/Actors.xml",                        "actor_turrets_rocket_turret")
	IMG("pak::bin\\graphics\\objects\\turrets\\uzi_turret.png",                               "Data/Atlases/Actors.xml",                        "actor_turrets_uzi_turret")
	IMG("pak::bin\\graphics\\objects\\vending machines\\food\\machine.png",                   "Data/Atlases/Actors.xml",                        "actor_vending_machine_food_machine")
	IMG("pak::bin\\graphics\\objects\\vending machines\\soda\\machine.png",                   "Data/Atlases/Actors.xml",                        "actor_vending_machine_soda_machine")
	IMG("pak::bin\\graphics\\objects\\weapons\\acid gun\\bullet_casing.png",                  "Data/Atlases/Weapons.xml",                       "actor_weapons_acid_gun_bullet_casing")
	IMG("pak::bin\\graphics\\objects\\weapons\\acid gun\\muzzle_flash.png",                   "Data/Atlases/Weapons.xml",                       "actor_weapons_acid_gun_muzzle_flash")
	IMG("pak::bin\\graphics\\objects\\weapons\\acid gun\\projectile.png",                     "Data/Atlases/Weapons.xml",                       "actor_weapons_acid_gun_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\ak47\\bullet_casing.png",                      "Data/Atlases/Weapons.xml",                       "actor_weapons_ak47_bullet_casing")
	IMG("pak::bin\\graphics\\objects\\weapons\\ak47\\muzzle_flash.png",                       "Data/Atlases/Weapons.xml",                       "actor_weapons_ak47_muzzle_flash")
	IMG("pak::bin\\graphics\\objects\\weapons\\ak47\\projectile.png",                         "Data/Atlases/Weapons.xml",                       "actor_weapons_ak47_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\bolt gun\\bullet_casing.png",                  "Data/Atlases/Weapons.xml",                       "actor_weapons_bolt_gun_bullet_casing")
	IMG("pak::bin\\graphics\\objects\\weapons\\bolt gun\\explosion_large.png",                "Data/Atlases/Weapons.xml",                       "actor_weapons_bolt_gun_explosion_large")
	IMG("pak::bin\\graphics\\objects\\weapons\\bolt gun\\explosion_small.png",                "Data/Atlases/Weapons.xml",                       "actor_weapons_bolt_gun_explosion_small")
	IMG("pak::bin\\graphics\\objects\\weapons\\bolt gun\\muzzle_flash.png",                   "Data/Atlases/Weapons.xml",                       "actor_weapons_bolt_gun_muzzle_flash")
	IMG("pak::bin\\graphics\\objects\\weapons\\bolt gun\\projectile.png",                     "Data/Atlases/Weapons.xml",                       "actor_weapons_bolt_gun_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\buff grenade\\beacon.png",                     "Data/Atlases/Weapons.xml",                       "actor_weapons_buff_grenade_beacon")
	IMG("pak::bin\\graphics\\objects\\weapons\\buff grenade\\buff_grenade_ammo.png",          "Data/Atlases/Weapons.xml",                       "actor_weapons_buff_grenade_buff_grenade_ammo")
	IMG("pak::bin\\graphics\\objects\\weapons\\buff grenade\\buff_grenade_damage.png",        "Data/Atlases/Weapons.xml",                       "actor_weapons_buff_grenade_buff_grenade_damage")
	IMG("pak::bin\\graphics\\objects\\weapons\\buff grenade\\buff_grenade_health.png",        "Data/Atlases/Weapons.xml",                       "actor_weapons_buff_grenade_buff_grenade_health")
	IMG("pak::bin\\graphics\\objects\\weapons\\buff grenade\\buff_grenade_price.png",         "Data/Atlases/Weapons.xml",                       "actor_weapons_buff_grenade_buff_grenade_price")
	IMG("pak::bin\\graphics\\objects\\weapons\\buff grenade\\buff_grenade_reload.png",        "Data/Atlases/Weapons.xml",                       "actor_weapons_buff_grenade_buff_grenade_reload")
	IMG("pak::bin\\graphics\\objects\\weapons\\buff grenade\\buff_grenade_rof.png",           "Data/Atlases/Weapons.xml",                       "actor_weapons_buff_grenade_buff_grenade_rof")
	IMG("pak::bin\\graphics\\objects\\weapons\\buff grenade\\buff_grenade_speed.png",         "Data/Atlases/Weapons.xml",                       "actor_weapons_buff_grenade_buff_grenade_speed")
	IMG("pak::bin\\graphics\\objects\\weapons\\buff grenade\\buff_grenade_xp.png",            "Data/Atlases/Weapons.xml",                       "actor_weapons_buff_grenade_buff_grenade_xp")
	IMG("pak::bin\\graphics\\objects\\weapons\\confetti\\projectile.png",                     "Data/Atlases/Weapons.xml",                       "actor_weapons_confetti_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\double barrel shotgun\\bullet_casing.png",     "Data/Atlases/Weapons.xml",                       "actor_weapons_double_barrel_shotgun_bullet_casing")
	IMG("pak::bin\\graphics\\objects\\weapons\\double barrel shotgun\\muzzle_flash.png",      "Data/Atlases/Weapons.xml",                       "actor_weapons_double_barrel_shotgun_muzzle_flash")
	IMG("pak::bin\\graphics\\objects\\weapons\\double barrel shotgun\\projectile.png",        "Data/Atlases/Weapons.xml",                       "actor_weapons_double_barrel_shotgun_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\flamethrower\\projectile.png",                 "Data/Atlases/Weapons.xml",                       "actor_weapons_flamethrower_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\flare gun\\bullet_casing.png",                 "Data/Atlases/Weapons.xml",                       "actor_weapons_flare_gun_bullet_casing")
	IMG("pak::bin\\graphics\\objects\\weapons\\flare gun\\muzzle_flash.png",                  "Data/Atlases/Weapons.xml",                       "actor_weapons_flare_gun_muzzle_flash")
	IMG("pak::bin\\graphics\\objects\\weapons\\flare gun\\projectile.png",                    "Data/Atlases/Weapons.xml",                       "actor_weapons_flare_gun_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\freeze ray\\freeze.png",                       "Data/Atlases/Weapons.xml",                       "actor_weapons_freeze_ray_freeze")
	IMG("pak::bin\\graphics\\objects\\weapons\\freeze ray\\projectile.png",                   "Data/Atlases/Weapons.xml",                       "actor_weapons_freeze_ray_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\generic\\smoke.png",                           "Data/Atlases/Weapons.xml",                       "actor_weapons_generic_smoke")
	IMG("pak::bin\\graphics\\objects\\weapons\\grenade launcher\\bullet_casing.png",          "Data/Atlases/Weapons.xml",                       "actor_weapons_grenade_launcher_bullet_casing")
	IMG("pak::bin\\graphics\\objects\\weapons\\grenade launcher\\explosion_extra_small.png",  "Data/Atlases/Weapons.xml",                       "actor_weapons_grenade_launcher_explosion_extra_small")
	IMG("pak::bin\\graphics\\objects\\weapons\\grenade launcher\\explosion_large.png",        "Data/Atlases/Weapons.xml",                       "actor_weapons_grenade_launcher_explosion_large")
	IMG("pak::bin\\graphics\\objects\\weapons\\grenade launcher\\explosion_small.png",        "Data/Atlases/Weapons.xml",                       "actor_weapons_grenade_launcher_explosion_small")
	IMG("pak::bin\\graphics\\objects\\weapons\\grenade launcher\\muzzle_flash.png",           "Data/Atlases/Weapons.xml",                       "actor_weapons_grenade_launcher_muzzle_flash")
	IMG("pak::bin\\graphics\\objects\\weapons\\grenade launcher\\projectile.png",             "Data/Atlases/Weapons.xml",                       "actor_weapons_grenade_launcher_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\laser cannon\\beam.png",                       "Data/Atlases/Weapons.xml",                       "actor_weapons_laser_cannon_beam")
	IMG("pak::bin\\graphics\\objects\\weapons\\laser cannon\\bullet_casing.png",              "Data/Atlases/Weapons.xml",                       "actor_weapons_laser_cannon_bullet_casing")
	IMG("pak::bin\\graphics\\objects\\weapons\\laser cannon\\muzzle_flash.png",               "Data/Atlases/Weapons.xml",                       "actor_weapons_laser_cannon_muzzle_flash")
	IMG("pak::bin\\graphics\\objects\\weapons\\laser cannon\\projectile.png",                 "Data/Atlases/Weapons.xml",                       "actor_weapons_laser_cannon_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\love cannon\\bullet_casing.png",               "Data/Atlases/Weapons.xml",                       "actor_weapons_love_cannon_bullet_casing")
	IMG("pak::bin\\graphics\\objects\\weapons\\love cannon\\muzzle_flash.png",                "Data/Atlases/Weapons.xml",                       "actor_weapons_love_cannon_muzzle_flash")
	IMG("pak::bin\\graphics\\objects\\weapons\\love cannon\\projectile.png",                  "Data/Atlases/Weapons.xml",                       "actor_weapons_love_cannon_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\love thrower\\projectile.png",                 "Data/Atlases/Weapons.xml",                       "actor_weapons_love_thrower_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\m16\\bullet_casing.png",                       "Data/Atlases/Weapons.xml",                       "actor_weapons_m16_bullet_casing")
	IMG("pak::bin\\graphics\\objects\\weapons\\m16\\muzzle_flash.png",                        "Data/Atlases/Weapons.xml",                       "actor_weapons_m16_muzzle_flash")
	IMG("pak::bin\\graphics\\objects\\weapons\\m16\\projectile.png",                          "Data/Atlases/Weapons.xml",                       "actor_weapons_m16_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\magnum\\bullet_casing.png",                    "Data/Atlases/Weapons.xml",                       "actor_weapons_magnum_bullet_casing")
	IMG("pak::bin\\graphics\\objects\\weapons\\magnum\\muzzle_flash.png",                     "Data/Atlases/Weapons.xml",                       "actor_weapons_magnum_muzzle_flash")
	IMG("pak::bin\\graphics\\objects\\weapons\\magnum\\projectile.png",                       "Data/Atlases/Weapons.xml",                       "actor_weapons_magnum_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\pistol\\bullet_casing.png",                    "Data/Atlases/Weapons.xml",                       "actor_weapons_pistol_bullet_casing")
	IMG("pak::bin\\graphics\\objects\\weapons\\pistol\\muzzle_flash.png",                     "Data/Atlases/Weapons.xml",                       "actor_weapons_pistol_muzzle_flash")
	IMG("pak::bin\\graphics\\objects\\weapons\\pistol\\projectile.png",                       "Data/Atlases/Weapons.xml",                       "actor_weapons_pistol_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\rocket launcher\\bullet_casing.png",           "Data/Atlases/Weapons.xml",                       "actor_weapons_rocket_launcher_bullet_casing")
	IMG("pak::bin\\graphics\\objects\\weapons\\rocket launcher\\explosion_large.png",         "Data/Atlases/Weapons.xml",                       "actor_weapons_rocket_launcher_explosion_large")
	IMG("pak::bin\\graphics\\objects\\weapons\\rocket launcher\\explosion_small.png",         "Data/Atlases/Weapons.xml",                       "actor_weapons_rocket_launcher_explosion_small")
	IMG("pak::bin\\graphics\\objects\\weapons\\rocket launcher\\muzzle_flash.png",            "Data/Atlases/Weapons.xml",                       "actor_weapons_rocket_launcher_muzzle_flash")
	IMG("pak::bin\\graphics\\objects\\weapons\\rocket launcher\\projectile.png",              "Data/Atlases/Weapons.xml",                       "actor_weapons_rocket_launcher_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\rpc\\bullet_casing.png",                       "Data/Atlases/Weapons.xml",                       "actor_weapons_rpc_bullet_casing")
	IMG("pak::bin\\graphics\\objects\\weapons\\rpc\\explosion_large.png",                     "Data/Atlases/Weapons.xml",                       "actor_weapons_rpc_explosion_large")
	IMG("pak::bin\\graphics\\objects\\weapons\\rpc\\explosion_small.png",                     "Data/Atlases/Weapons.xml",                       "actor_weapons_rpc_explosion_small")
	IMG("pak::bin\\graphics\\objects\\weapons\\rpc\\muzzle_flash.png",                        "Data/Atlases/Weapons.xml",                       "actor_weapons_rpc_muzzle_flash")
	IMG("pak::bin\\graphics\\objects\\weapons\\rpc\\projectile.png",                          "Data/Atlases/Weapons.xml",                       "actor_weapons_rpc_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\scatter gun\\bullet_casing.png",               "Data/Atlases/Weapons.xml",                       "actor_weapons_scatter_gun_bullet_casing")
	IMG("pak::bin\\graphics\\objects\\weapons\\scatter gun\\muzzle_flash.png",                "Data/Atlases/Weapons.xml",                       "actor_weapons_scatter_gun_muzzle_flash")
	IMG("pak::bin\\graphics\\objects\\weapons\\scatter gun\\projectile.png",                  "Data/Atlases/Weapons.xml",                       "actor_weapons_scatter_gun_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\shock rifle\\beam.png",                        "Data/Atlases/Weapons.xml",                       "actor_weapons_shock_rifle_beam")
	IMG("pak::bin\\graphics\\objects\\weapons\\shock rifle\\bullet_casing.png",               "Data/Atlases/Weapons.xml",                       "actor_weapons_shock_rifle_bullet_casing")
	IMG("pak::bin\\graphics\\objects\\weapons\\shock rifle\\muzzle_flash.png",                "Data/Atlases/Weapons.xml",                       "actor_weapons_shock_rifle_muzzle_flash")
	IMG("pak::bin\\graphics\\objects\\weapons\\shock rifle\\projectile.png",                  "Data/Atlases/Weapons.xml",                       "actor_weapons_shock_rifle_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\shotgun\\bullet_casing.png",                   "Data/Atlases/Weapons.xml",                       "actor_weapons_shotgun_bullet_casing")
	IMG("pak::bin\\graphics\\objects\\weapons\\shotgun\\muzzle_flash.png",                    "Data/Atlases/Weapons.xml",                       "actor_weapons_shotgun_muzzle_flash")
	IMG("pak::bin\\graphics\\objects\\weapons\\shotgun\\projectile.png",                      "Data/Atlases/Weapons.xml",                       "actor_weapons_shotgun_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\spaz\\bullet_casing.png",                      "Data/Atlases/Weapons.xml",                       "actor_weapons_spaz_bullet_casing")
	IMG("pak::bin\\graphics\\objects\\weapons\\spaz\\muzzle_flash.png",                       "Data/Atlases/Weapons.xml",                       "actor_weapons_spaz_muzzle_flash")
	IMG("pak::bin\\graphics\\objects\\weapons\\spaz\\projectile.png",                         "Data/Atlases/Weapons.xml",                       "actor_weapons_spaz_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\sticky launcher\\bullet_casing.png",           "Data/Atlases/Weapons.xml",                       "actor_weapons_sticky_launcher_bullet_casing")
	IMG("pak::bin\\graphics\\objects\\weapons\\sticky launcher\\explosion_large.png",         "Data/Atlases/Weapons.xml",                       "actor_weapons_sticky_launcher_explosion_large")
	IMG("pak::bin\\graphics\\objects\\weapons\\sticky launcher\\explosion_small.png",         "Data/Atlases/Weapons.xml",                       "actor_weapons_sticky_launcher_explosion_small")
	IMG("pak::bin\\graphics\\objects\\weapons\\sticky launcher\\muzzle_flash.png",            "Data/Atlases/Weapons.xml",                       "actor_weapons_sticky_launcher_muzzle_flash")
	IMG("pak::bin\\graphics\\objects\\weapons\\sticky launcher\\projectile.png",              "Data/Atlases/Weapons.xml",                       "actor_weapons_sticky_launcher_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\tripmine\\explosion_large.png",                "Data/Atlases/Weapons.xml",                       "actor_weapons_tripmine_explosion_large")
	IMG("pak::bin\\graphics\\objects\\weapons\\tripmine\\explosion_small.png",                "Data/Atlases/Weapons.xml",                       "actor_weapons_tripmine_explosion_small")
	IMG("pak::bin\\graphics\\objects\\weapons\\tripmine\\projectile.png",                     "Data/Atlases/Weapons.xml",                       "actor_weapons_tripmine_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\uzi\\bullet_casing.png",                       "Data/Atlases/Weapons.xml",                       "actor_weapons_uzi_bullet_casing")
	IMG("pak::bin\\graphics\\objects\\weapons\\uzi\\muzzle_flash.png",                        "Data/Atlases/Weapons.xml",                       "actor_weapons_uzi_muzzle_flash")
	IMG("pak::bin\\graphics\\objects\\weapons\\uzi\\projectile.png",                          "Data/Atlases/Weapons.xml",                       "actor_weapons_uzi_projectile")
	IMG("pak::bin\\graphics\\objects\\weapons\\winchester rifle\\bullet_casing.png",          "Data/Atlases/Weapons.xml",                       "actor_weapons_winchester_rifle_bullet_casing")
	IMG("pak::bin\\graphics\\objects\\weapons\\winchester rifle\\muzzle_flash.png",           "Data/Atlases/Weapons.xml",                       "actor_weapons_winchester_rifle_muzzle_flash")
	IMG("pak::bin\\graphics\\objects\\weapons\\winchester rifle\\projectile.png",             "Data/Atlases/Weapons.xml",                       "actor_weapons_winchester_rifle_projectile")
	IMG("pak::bin\\graphics\\objects\\weather\\default.png",                                  "Data/Atlases/Actors.xml",                        "actor_weather_default")
	IMG("pak::bin\\graphics\\objects\\weather\\fogu.png",                                     "Data/Atlases/Actors.xml",                        "actor_weather_fogu")
	IMG("pak::bin\\graphics\\objects\\weather\\nozone.png",                                   "Data/Atlases/Actors.xml",                        "actor_weather_nozone")
	IMG("pak::bin\\graphics\\objects\\weather\\rain\\rain.png",                               "Data/Atlases/Actors.xml",                        "actor_weather_rain")
	IMG("pak::bin\\graphics\\objects\\zombie zone\\default.png",                              "Data/Atlases/Actors.xml",                        "actor_zombie_zone_default")
	IMG("pak::bin\\graphics\\objects\\zombie zone\\nofodderzombiezone.png",                   "Data/Atlases/Actors.xml",                        "actor_zombie_zone_nofodderzombiezone")
	IMG("pak::bin\\graphics\\objects\\zombie zone\\single.png",                               "Data/Atlases/Actors.xml",                        "actor_zombie_zone_single")
	IMG("pak::bin\\graphics\\overlays\\cracks.png",                                           "Data/Atlases/Overlays.xml",                      "overlay_cracks")
	IMG("pak::bin\\graphics\\overlays\\dark.png",                                             "Data/Atlases/Overlays.xml",                      "overlay_dark")
	IMG("pak::bin\\graphics\\overlays\\fog.png",                                              "Data/Atlases/Overlays.xml",                      "overlay_fog")
	IMG("pak::bin\\graphics\\overlays\\fog_2.png",                                            "Data/Atlases/Overlays.xml",                      "overlay_fog_2")
	IMG("pak::bin\\graphics\\overlays\\full_dark.png",                                        "Data/Atlases/Overlays.xml",                      "overlay_full_dark")
	IMG("pak::bin\\graphics\\overlays\\rain.png",                                             "Data/Atlases/Overlays.xml",                      "overlay_rain")
	IMG("pak::bin\\graphics\\particles\\acid.png",                                            "Data/Atlases/Particles.xml",                     "particle_acid")
	IMG("pak::bin\\graphics\\particles\\fire.png",                                            "Data/Atlases/Particles.xml",                     "particle_fire")
	IMG("pak::bin\\graphics\\particles\\smoke.png",                                           "Data/Atlases/Particles.xml",                     "particle_smoke")
	IMG("pak::bin\\graphics\\tutorials\\armour.png",                                          "Data/Atlases/System.xml",                        "tutorial_armour")
	IMG("pak::bin\\graphics\\tutorials\\bag.png",                                             "Data/Atlases/System.xml",                        "tutorial_bag")
	IMG("pak::bin\\graphics\\tutorials\\baseball bat.png",                                    "Data/Atlases/System.xml",                        "tutorial_baseball bat")
	IMG("pak::bin\\graphics\\tutorials\\coin.png",                                            "Data/Atlases/System.xml",                        "tutorial_coin")
	IMG("pak::bin\\graphics\\tutorials\\default.png",                                         "Data/Atlases/System.xml",                        "tutorial_default")
	IMG("pak::bin\\graphics\\tutorials\\double barrel shotgun.png",                           "Data/Atlases/System.xml",                        "tutorial_double barrel shotgun")
	IMG("pak::bin\\graphics\\tutorials\\explosive ammo.png",                                  "Data/Atlases/System.xml",                        "tutorial_explosive ammo")
	IMG("pak::bin\\graphics\\tutorials\\fire_turret.png",                                     "Data/Atlases/System.xml",                        "tutorial_fire_turret")
	IMG("pak::bin\\graphics\\tutorials\\flamethrower.png",                                    "Data/Atlases/System.xml",                        "tutorial_flamethrower")
	IMG("pak::bin\\graphics\\tutorials\\flare gun.png",                                       "Data/Atlases/System.xml",                        "tutorial_flare gun")
	IMG("pak::bin\\graphics\\tutorials\\freeze ammo.png",                                     "Data/Atlases/System.xml",                        "tutorial_freeze ammo")
	IMG("pak::bin\\graphics\\tutorials\\freeze ray.png",                                      "Data/Atlases/System.xml",                        "tutorial_freeze ray")
	IMG("pak::bin\\graphics\\tutorials\\gem.png",                                             "Data/Atlases/System.xml",                        "tutorial_gem")
	IMG("pak::bin\\graphics\\tutorials\\grenade launcher.png",                                "Data/Atlases/System.xml",                        "tutorial_grenade launcher")
	IMG("pak::bin\\graphics\\tutorials\\health.png",                                          "Data/Atlases/System.xml",                        "tutorial_health")
	IMG("pak::bin\\graphics\\tutorials\\help.png",                                            "Data/Atlases/System.xml",                        "tutorial_help")
	IMG("pak::bin\\graphics\\tutorials\\incendiary ammo.png",                                 "Data/Atlases/System.xml",                        "tutorial_incendiary ammo")
	IMG("pak::bin\\graphics\\tutorials\\info.png",                                            "Data/Atlases/System.xml",                        "tutorial_info")
	IMG("pak::bin\\graphics\\tutorials\\interactable object.png",                             "Data/Atlases/System.xml",                        "tutorial_interactable object")
	IMG("pak::bin\\graphics\\tutorials\\key.png",                                             "Data/Atlases/System.xml",                        "tutorial_key")
	IMG("pak::bin\\graphics\\tutorials\\laser cannon.png",                                    "Data/Atlases/System.xml",                        "tutorial_laser cannon")
	IMG("pak::bin\\graphics\\tutorials\\love cannon.png",                                     "Data/Atlases/System.xml",                        "tutorial_love cannon")
	IMG("pak::bin\\graphics\\tutorials\\magnum.png",                                          "Data/Atlases/System.xml",                        "tutorial_magnum")
	IMG("pak::bin\\graphics\\tutorials\\rankup.png",                                          "Data/Atlases/System.xml",                        "tutorial_rankup")
	IMG("pak::bin\\graphics\\tutorials\\rocket launcher.png",                                 "Data/Atlases/System.xml",                        "tutorial_rocket launcher")
	IMG("pak::bin\\graphics\\tutorials\\rocket_turret.png",                                   "Data/Atlases/System.xml",                        "tutorial_rocket_turret")
	IMG("pak::bin\\graphics\\tutorials\\rpc.png",                                             "Data/Atlases/System.xml",                        "tutorial_rpc")
	IMG("pak::bin\\graphics\\tutorials\\scatter gun.png",                                     "Data/Atlases/System.xml",                        "tutorial_scatter gun")
	IMG("pak::bin\\graphics\\tutorials\\shotgun.png",                                         "Data/Atlases/System.xml",                        "tutorial_shotgun")
	IMG("pak::bin\\graphics\\tutorials\\spaz.png",                                            "Data/Atlases/System.xml",                        "tutorial_spaz")
	IMG("pak::bin\\graphics\\tutorials\\sticky launcher.png",                                 "Data/Atlases/System.xml",                        "tutorial_sticky launcher")
	IMG("pak::bin\\graphics\\tutorials\\tripmine.png",                                        "Data/Atlases/System.xml",                        "tutorial_tripmine")
	IMG("pak::bin\\graphics\\tutorials\\turret.png",                                          "Data/Atlases/System.xml",                        "tutorial_turret")
	IMG("pak::bin\\graphics\\tutorials\\update.png",                                          "Data/Atlases/System.xml",                        "tutorial_update")
	IMG("pak::bin\\graphics\\tutorials\\uzi.png",                                             "Data/Atlases/System.xml",                        "tutorial_uzi")
	IMG("pak::bin\\graphics\\tutorials\\welcome.png",                                         "Data/Atlases/System.xml",                        "tutorial_welcome")
#undef IMG
}

bool LegacyMapConverter::Convert(const char* from_path, const char* to_path)
{
	Stream* stream = StreamFactory::Open(from_path, StreamMode::Read);
	if (stream == NULL)
	{
		DBG_LOG("Failed to load legacy map '%s', could not open file.", from_path);
		return false;
	}

	// Read header information.
	const char* header = stream->ReadString(5);
	std::string header_str = header;
	if (header_str != "ZGMAP")
	{
		DBG_LOG("Failed to load legacy map '%s'. Invalid header.", from_path);
		SAFE_DELETE(stream);
		return false;
	}
	
	// Read version information.
	int map_version = stream->Read<int>();
	if (map_version < 5)
	{
		DBG_LOG("Failed to load legacy map '%s'. Version is lower than supported version (9).", from_path);
		SAFE_DELETE(stream);
		return false;
	}
	
	// Read depth
	int depth = stream->Read<int>();
	if (depth != 5)
	{
		DBG_LOG("Failed to load legacy map '%s'. Depth is an unsupported value.", from_path);
		SAFE_DELETE(stream);
		return false;
	}

	// Create a map file.
	MapFile* map_file = new MapFile();

	// Header values.
	MapFileHeaderBlock* header_block = map_file->New_Block<MapFileHeaderBlock>();
	header_block->Short_Name = Platform::Get()->Extract_Basename(to_path);

	// Read attributes.
	AtlasHandle* tileset = NULL;
	int tile_width = 0;
	int tile_height = 0;
	std::string game_mode = "Wave";

	int attribute_count = stream->Read<int>();
	for (int i = 0; i < attribute_count; i++)
	{
		std::string name			= stream->ReadString(stream->Read<int>());
		std::string mode			= stream->ReadString(stream->Read<int>());
		std::string string_value	= "";
		float		float_value		= 0;
		int			int_value		= 0;

		if (mode == "image")	string_value = stream->ReadString(stream->Read<int>());
		if (mode == "file")		string_value = stream->ReadString(stream->Read<int>());
		if (mode == "string")	string_value = stream->ReadString(stream->Read<int>());
		if (mode == "float")	float_value = stream->Read<float>();
		if (mode == "int")		int_value = stream->Read<int>();
		if (mode == "combo")	int_value = stream->Read<int>();

		if (name == "Name")				{ header_block->Long_Name		= string_value; }
		if (name == "Author")			{ header_block->Author			= string_value; }
		if (name == "Description")		{ header_block->Description		= string_value; }
		if (name == "MinPlayers")		{ header_block->Min_Players		= int_value; }
		if (name == "MaxPlayers")		{ header_block->Max_Players		= int_value; }
		if (name == "GameModeName")		{ game_mode = Platform::Get()->Extract_Basename(string_value.c_str()); }
		if (name == "BGMPath")			
		{ 
			if (string_value == "")
			{
				header_block->Music_Path = "music_zombroni";
			}
			else
			{
				header_block->Music_Path = "music_" + Platform::Get()->Extract_Basename(string_value.c_str()); 
			}
		}
		if (name == "LoadingSlogan")	{ header_block->Loading_Slogan	= string_value; }

		if (name == "Tileset")
		{
			string_value			= StringHelper::Lowercase(string_value.c_str());
			unsigned int	hash	= StringHelper::Hash(string_value.c_str());					
			const char*		mapped	= "";

			if (m_tileset_map.Get(hash, mapped))
			{
				header_block->Tileset_Name = mapped;
			}
			else
			{
				DBG_LOG("Failed to load legacy map '%s'. Unknown tileset '%s'.", string_value.c_str());
				SAFE_DELETE(stream);
				return false;
			}
		}
		else
		{
			DBG_LOG("Ignoring unused property '%s' in legacy map.", name.c_str());
		}
	}

	// Read preview.
	if (stream->Read<char>() != 0)
	{
		int size = stream->Read<int>();

		char* buffer = new char[size];
		stream->ReadBuffer(buffer, 0, size);

		std::string dir		 = Platform::Get()->Extract_Directory(to_path);
		std::string basename = Platform::Get()->Extract_Basename(to_path);

		std::string preview = StringHelper::Format("%s/preview.png", dir.c_str());
		Stream* s = StreamFactory::Open(preview.c_str(), StreamMode::Write);
		s->WriteBuffer(buffer, 0, size);
		s->Close();
		
		preview = StringHelper::Replace(preview.c_str(), "../Data/Base/", "Data/");

		header_block->Preview_Path = preview;
	}

	// Read object count.
	int object_count = stream->Read<int>();

	// Read in layer header information.
	for (int i = 0; i < depth; i++)
	{
		std::string layer_type_id = stream->ReadString(stream->Read<int>());
		
		MapFileLayerBlock* layer_block = map_file->New_Block<MapFileLayerBlock>();

		int layer_index		= stream->Read<int>();
		layer_block->Width	= stream->Read<int>();
		layer_block->Height	= stream->Read<int>();
		float parallax_x	= stream->Read<float>();
		float parallax_y	= stream->Read<float>();
		bool is_tiled		= (stream->Read<unsigned char>() != 0);
	
		header_block->Width = layer_block->Width;
		header_block->Height = layer_block->Height;

		layer_block->Tiles  = new MapTile[layer_block->Width * layer_block->Height];

		// Read in tile information.
		for (int x = 0; x < layer_block->Width; x++)
		{
			for (int y = 0; y < layer_block->Height; y++)
			{
				MapTile& tile			= layer_block->Tiles[(y * layer_block->Width) + x];
				tile.Frame				= stream->Read<int>();
				if (tile.Frame == 65535)
				{
					tile.Frame			= 0xFFFF;
				}
				tile.TileColor.A			= (int)(stream->Read<float>() * 255.0f);
				tile.TileColor.R			= stream->Read<unsigned char>();
				tile.TileColor.G			= stream->Read<unsigned char>();
				tile.TileColor.B			= stream->Read<unsigned char>();
				tile.CollisionFrame			= stream->Read<int>();
				tile.Collision				= (MapTileCollisionType::Type)stream->Read<int>();
				if (tile.Collision == 255)
				{
					tile.Collision = MapTileCollisionType::None;
				}
				else
				{
					tile.Collision = (MapTileCollisionType::Type)(((int)tile.Collision) + 1);
				}
				tile.FlipHorizontal	= (stream->Read<unsigned char>() != 0);
				tile.FlipVertical		= (stream->Read<unsigned char>() != 0);
			}
		}
	}

	// Create game-mode object.
	std::string game_mode_class = StringHelper::Replace(game_mode.c_str(), " ", "_") + "_Game_Mode";
		
	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMLinkedSymbol* gm_sym = vm->Find_Class(game_mode_class.c_str());
	if (gm_sym == NULL)
	{
		DBG_LOG("Could not find '%s' game mode, using wave game mode instead.", game_mode_class.c_str());
		game_mode_class = "Wave_Game_Mode";
		gm_sym = vm->Find_Class(game_mode_class.c_str());
	}
	if (gm_sym != NULL)
	{
		// Create an actor.
		ScriptedActor* actor = new ScriptedActor(vm, gm_sym);
		actor->Setup_Script();

		actor->Set_Position(Vector3(0, 0, 0));
		actor->Set_Layer(2);
		actor->Set_Bounding_Box(Rect2D(0, 0, 16, 16));

		// Attach a drawable component.
		SpriteComponent* sprite = actor->Get_Component<SpriteComponent*>();
		sprite->Set_Visible(false);
		sprite->Set_Editor_Visible(true);
		sprite->Set_Color(Color(255.0f, 255.0f, 255.0f, 255.0f));
		sprite->Set_Frame(ResourceFactory::Get()->Get_Atlas_Frame("actor_game_mode_default_0"));

		// Save object block.
		MapFileObjectBlock* object_block = map_file->New_Block<MapFileObjectBlock>();
		object_block->Class_Name = game_mode_class.c_str();
			
		BinaryStream bs;
		int output_version = 0;
		CVMContextLock lock = vm->Set_Context(actor->Get_Script_Context());
		actor->Get_Script_Object().Get()->Serialize(&bs, CVMObjectSerializeFlags::Full, &output_version);		
		object_block->Serialized_Data_Version = output_version;
		object_block->Serialized_Data.Set(bs.Data(), (int)bs.Length());				
		bs.Close();

		// Delete everything.
		SAFE_DELETE(actor);
	}

	// Read in objects.
	for (int i = 0; i < object_count; i++)
	{
		Load_Entity(map_file, map_version, stream, true);
	}

	// Save output map file.
	map_file->Save(to_path);

	// Tidy up.
	SAFE_DELETE(stream);
	
	DBG_LOG("Successfully converted legacy map '%s'.", from_path);
	return true;
}

void LegacyMapConverter::Load_Entity(MapFile* map, int map_version, Stream* stream, bool with_size)
{
	std::string entity_id	= stream->ReadString(stream->Read<int>());	
	int entity_size			= with_size == true ? stream->Read<int>() : 0;
	int expected_end		= stream->Position() + entity_size;

	if (entity_id == "TDecal")
	{
		Load_Decal(map, map_version, stream);
	}
	else if (entity_id == "TPlayerStart")
	{
		Load_Player_Start(map, map_version, stream);
	}
	else
	{		
		DBG_ASSERT_STR(with_size, "Unable to continue, cannot load mandatory entity '%s'.", entity_id.c_str());
		DBG_LOG("Skipping unsupported legacy entity type '%s'.", entity_id.c_str());

		stream->Seek(expected_end);
	}

	int end_position = stream->Position();
	DBG_ASSERT_STR(with_size == false || end_position == expected_end, "Entire entity structure has not been read.");
}

void LegacyMapConverter::Load_Decal(MapFile* map, int map_version, Stream* stream)
{
	std::string image_url					= stream->ReadString(stream->Read<int>());
	std::string default_animation_name		= stream->ReadString(stream->Read<int>());
	bool		destroy_on_anim_end			= (stream->Read<unsigned char>() != 0);
	
	if (map_version >= 2)
	{
		float	fade_distance_min			= stream->Read<float>();
		float	fade_distance_max			= stream->Read<float>();
	}

	if (map_version >= 6)
	{
		bool	fade_on_touch				= (stream->Read<unsigned char>() != 0);
	}

	std::string name						= stream->ReadString(stream->Read<int>());
	std::string event						= stream->ReadString(stream->Read<int>());

	if (map_version > 1)
	{
		std::string dynamic_group			= stream->ReadString(stream->Read<int>());
		std::string expect_dynamic_group	= stream->ReadString(stream->Read<int>());
	}

	float		x							= stream->Read<float>();	
	float		y							= stream->Read<float>();	
	float		z							= stream->Read<float>();	
	int			layer						= stream->Read<unsigned char>();
	bool		is_solid					= (stream->Read<unsigned char>() != 0);
	int			collision_box_x				= stream->Read<int>();
	int			collision_box_y				= stream->Read<int>();
	int			collision_box_w				= stream->Read<int>();
	int			collision_box_h				= stream->Read<int>();

	int	bounding_box_x = 0;
	int	bounding_box_y = 0;
	int	bounding_box_w = 0;
	int	bounding_box_h = 0;
	if (map_version > 1)
	{
		bounding_box_x					= stream->Read<int>();
		bounding_box_y					= stream->Read<int>();
		bounding_box_w					= stream->Read<int>();
		bounding_box_h					= stream->Read<int>();
	}

	int depth_bias = 0;
	if (map_version > 2)
	{
		depth_bias						= stream->Read<int>();
	}

	int			frameIndex					= stream->Read<unsigned short>();
	float		scale_x						= stream->Read<float>();	
	float		scale_y						= stream->Read<float>();	
	float		angle						= stream->Read<float>();	
	int			red							= stream->Read<unsigned char>();
	int			green						= stream->Read<unsigned char>();
	int			blue						= stream->Read<unsigned char>();
	float		alpha						= stream->Read<float>();
	int			blend_mode					= stream->Read<unsigned char>();
	int			child_count					= stream->Read<int>();

	// Try and decode the frame.
	const char* frame_name = "";
	std::string final_name = StringHelper::Lowercase(image_url.c_str());
	unsigned int hash = StringHelper::Hash(final_name.c_str());

	// INI rather than png file?
	if (!m_image_map.Get(hash, frame_name))
	{
		final_name = StringHelper::Replace(final_name.c_str(), ".ini", ".png");
		hash = StringHelper::Hash(final_name.c_str());
	}
	if (!m_image_map.Get(hash, frame_name))
	{
		final_name = StringHelper::Replace(final_name.c_str(), ".png", ".ini");
		hash = StringHelper::Hash(final_name.c_str());
	}

	if (m_image_map.Get(hash, frame_name))
	{
		// Try getting indexed frame first.
		std::string best_frame = StringHelper::Format("%s_%i", frame_name, frameIndex);
		AtlasFrame* frame = ResourceFactory::Get()->Get_Atlas_Frame(best_frame.c_str());

		// If not try to get non-indexed frame.
		if (frame == NULL)
		{
			frame = ResourceFactory::Get()->Get_Atlas_Frame(frame_name);
		}

		// Annnnd, we fail.
		if (frame != NULL)
		{
			// Adjust position by scale.
			bool flipped_h = false;
			bool flipped_v = false;
			
			if (scale_x < 1.0f)
			{
				scale_x = fabs(scale_x);
				x -= frame->Rect.Width;
				flipped_h = true;
			}
			if (scale_y < 1.0f)
			{
				scale_y = fabs(scale_y);
				y -= frame->Rect.Height;
				flipped_v = true;
			}
			
			// Grab the correct VM.
			EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
			CVMLinkedSymbol* decal_sym = vm->Find_Class("Decal");

			// Create an actor.
			ScriptedActor* actor = new ScriptedActor(vm, decal_sym);
			actor->Setup_Script();

			actor->Set_Scale(Vector3(scale_x, scale_y, 1.0f));
			actor->Set_Position(Vector3(x, y, z));
			actor->Set_Rotation(Vector3(0, 0, angle));
			actor->Set_Layer(layer);
			actor->Set_Depth_Bias((float)depth_bias);
			actor->Set_Bounding_Box(Rect2D(bounding_box_x, bounding_box_y, bounding_box_w, bounding_box_h));
			
			// Attach a drawable component.
			SpriteComponent* sprite = actor->Get_Component<SpriteComponent*>();
			sprite->Set_Color(Color((float)red, (float)green, (float)blue, alpha * 255.0f));
			sprite->Set_Frame(frame);
			sprite->Set_Flip(flipped_h, flipped_v);

			// Save object block.
			MapFileObjectBlock* object_block = map->New_Block<MapFileObjectBlock>();
			object_block->Class_Name = "Decal";
			
			BinaryStream bs;
			int output_version = 0;
			CVMContextLock lock = vm->Set_Context(actor->Get_Script_Context());
			actor->Get_Script_Object().Get()->Serialize(&bs, CVMObjectSerializeFlags::Full, &output_version);		
			object_block->Serialized_Data_Version = output_version;
			object_block->Serialized_Data.Set(bs.Data(), (int)bs.Length());				
			bs.Close();

			// Delete everything.
			SAFE_DELETE(actor);

			DBG_LOG("Loaded decal: %s", image_url.c_str());
		}
		else
		{			
			DBG_LOG("Mapped frame for object appears to be invalid: %s", frame_name);
		}
	}
	else
	{
		DBG_LOG("Ignored decal, could not find image: %s", image_url.c_str());
	}

	// Load children.
	for (int i = 0; i < child_count; i++)
	{
		Load_Entity(map, map_version, stream, false);
	}
}
void LegacyMapConverter::Load_Player_Start(MapFile* map, int map_version, Stream* stream)
{
	int			player_id					= stream->Read<int>();
	int			direction					= stream->Read<unsigned char>();
	int			map_start					= stream->Read<unsigned char>();

	std::string name						= stream->ReadString(stream->Read<int>());
	std::string event						= stream->ReadString(stream->Read<int>());

	if (map_version > 1)
	{
		std::string dynamic_group			= stream->ReadString(stream->Read<int>());
		std::string expect_dynamic_group	= stream->ReadString(stream->Read<int>());
	}

	float		x							= stream->Read<float>();	
	float		y							= stream->Read<float>();	
	float		z							= stream->Read<float>();	
	int			layer						= stream->Read<unsigned char>();
	bool		is_solid					= (stream->Read<unsigned char>() != 0);
	int			collision_box_x				= stream->Read<int>();
	int			collision_box_y				= stream->Read<int>();
	int			collision_box_w				= stream->Read<int>();
	int			collision_box_h				= stream->Read<int>();

	int	bounding_box_x = 0;
	int	bounding_box_y = 0;
	int	bounding_box_w = 0;
	int	bounding_box_h = 0;
	if (map_version > 1)
	{
		bounding_box_x					= stream->Read<int>();
		bounding_box_y					= stream->Read<int>();
		bounding_box_w					= stream->Read<int>();
		bounding_box_h					= stream->Read<int>();
	}

	int depth_bias = 0;
	if (map_version > 2)
	{
		depth_bias						= stream->Read<int>();
	}

	int			frame						= stream->Read<unsigned short>();
	float		scale_x						= stream->Read<float>();	
	float		scale_y						= stream->Read<float>();	
	float		angle						= stream->Read<float>();	
	int			red							= stream->Read<unsigned char>();
	int			green						= stream->Read<unsigned char>();
	int			blue						= stream->Read<unsigned char>();
	float		alpha						= stream->Read<float>();
	int			blend_mode					= stream->Read<unsigned char>();
	int			child_count					= stream->Read<int>();

	// Grab the correct VM.
	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMLinkedSymbol* decal_sym = vm->Find_Class("Player_Spawn");
	CVMLinkedSymbol* team_sym = vm->Find_Variable(decal_sym, "Team_Index");
	CVMLinkedSymbol* direction_sym = vm->Find_Variable(decal_sym, "Spawn_Direction");

	// Create an actor.
	ScriptedActor* actor = new ScriptedActor(vm, decal_sym);
	actor->Setup_Script();
	
	CVMContextLock lock = vm->Set_Context(actor->Get_Script_Context());
	CVMValue team_value(0);
	CVMValue dir_value(Directions::S);
	vm->Set_Field(team_sym, actor->Get_Script_Object(), team_value); 
	vm->Set_Field(direction_sym, actor->Get_Script_Object(), dir_value); 
	
	actor->Set_Enabled(map_start != 0);
	actor->Set_Scale(Vector3(scale_x, scale_y, 1.0f));
	actor->Set_Position(Vector3(x, y, z));
	actor->Set_Rotation(Vector3(0, 0, angle));
	actor->Set_Layer(layer);
	actor->Set_Depth_Bias((float)depth_bias);
	actor->Set_Bounding_Box(Rect2D(bounding_box_x, bounding_box_y, bounding_box_w, bounding_box_h));

	// Save object block.
	MapFileObjectBlock* object_block = map->New_Block<MapFileObjectBlock>();
	object_block->Class_Name = "Player_Spawn";
			
	BinaryStream bs;
	int output_version = 0;
	actor->Get_Script_Object().Get()->Serialize(&bs, CVMObjectSerializeFlags::Full, &output_version);		
	object_block->Serialized_Data_Version = output_version;
	object_block->Serialized_Data.Set(bs.Data(), (int)bs.Length());				
	bs.Close();

	// Delete everything.
	SAFE_DELETE(actor);

	DBG_LOG("Loaded player start: %s", name.c_str());

	// Load children.
	for (int i = 0; i < child_count; i++)
	{
		Load_Entity(map, map_version, stream, false);
	}
}