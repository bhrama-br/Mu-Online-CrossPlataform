cmake_minimum_required(VERSION 3.22.1)

project(mu_android_bootstrap LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MU_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../../../)

add_library(mu_android_bootstrap SHARED
    android_bootstrap.cpp
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c

    # ---- Existing bootstrap sources ----
    ${MU_ROOT_DIR}/source/CCRC32.cpp
    ${MU_ROOT_DIR}/source/CriticalSection.cpp
    ${MU_ROOT_DIR}/source/Sprite.cpp
    ${MU_ROOT_DIR}/source/Win.cpp
    ${MU_ROOT_DIR}/source/WinEx.cpp
    ${MU_ROOT_DIR}/source/Button.cpp
    ${MU_ROOT_DIR}/source/GaugeBar.cpp
    ${MU_ROOT_DIR}/source/Input.cpp
    ${MU_ROOT_DIR}/source/PList.cpp
    ${MU_ROOT_DIR}/source/LoginMainWin.cpp
    ${MU_ROOT_DIR}/source/CharSelMainWin.cpp
    ${MU_ROOT_DIR}/source/CharMakeWin.cpp
    ${MU_ROOT_DIR}/source/LoadData.cpp
    ${MU_ROOT_DIR}/source/PacketManager.cpp
    ${MU_ROOT_DIR}/source/Protect.cpp
    ${MU_ROOT_DIR}/source/ServerSelWin.cpp
    ${MU_ROOT_DIR}/source/ServerGroup.cpp
    ${MU_ROOT_DIR}/source/ServerInfo.cpp
    ${MU_ROOT_DIR}/source/ServerListManager.cpp
    ${MU_ROOT_DIR}/source/LoginWin.cpp
    ${MU_ROOT_DIR}/source/UIMng.cpp
    ${MU_ROOT_DIR}/source/Time/Timer.cpp
    ${MU_ROOT_DIR}/source/Math/ZzzMathLib.cpp
    ${MU_ROOT_DIR}/source/TextureScript.cpp
    ${MU_ROOT_DIR}/source/ZzzOpenglUtil.cpp
    ${MU_ROOT_DIR}/source/ZzzBMD.cpp
    ${MU_ROOT_DIR}/source/ZzzOpenData.cpp
    ${MU_ROOT_DIR}/source/ZzzCharacter.cpp
    ${MU_ROOT_DIR}/source/w_ObjectInfo.cpp
    ${MU_ROOT_DIR}/source/w_CharacterInfo.cpp

    # ---- Engine core (NEW) ----
    ${MU_ROOT_DIR}/source/WSctlc.cpp
    ${MU_ROOT_DIR}/source/WSclient.cpp
    ${MU_ROOT_DIR}/source/ZzzScene.cpp
    ${MU_ROOT_DIR}/source/ZzzObject.cpp
    ${MU_ROOT_DIR}/source/ZzzInterface.cpp
    ${MU_ROOT_DIR}/source/ZzzInventory.cpp
    ${MU_ROOT_DIR}/source/ZzzLodTerrain.cpp
    ${MU_ROOT_DIR}/source/ZzzTexture.cpp
    ${MU_ROOT_DIR}/source/ZzzAI.cpp
    ${MU_ROOT_DIR}/source/ZzzInfomation.cpp
    ${MU_ROOT_DIR}/source/ZzzEffect.cpp
    ${MU_ROOT_DIR}/source/ZzzEffectParticle.cpp
    ${MU_ROOT_DIR}/source/ZzzEffectJoint.cpp
    ${MU_ROOT_DIR}/source/ZzzEffectMagicSkill.cpp
    ${MU_ROOT_DIR}/source/ZzzEffectBlurSpark.cpp
    ${MU_ROOT_DIR}/source/ZzzEffectFireLeave.cpp
    ${MU_ROOT_DIR}/source/ZzzEffectNoUse.cpp
    ${MU_ROOT_DIR}/source/ZzzEffectPoint.cpp
    ${MU_ROOT_DIR}/source/ZzzEffectPointer.cpp
    ${MU_ROOT_DIR}/source/zzzeffectsprite.cpp
    ${MU_ROOT_DIR}/source/zzzpath.cpp
    ${MU_ROOT_DIR}/source/zzzMixInvetory.cpp
    ${MU_ROOT_DIR}/source/GlobalBitmap.cpp
    ${MU_ROOT_DIR}/source/PhysicsManager.cpp
    ${MU_ROOT_DIR}/source/ShadowVolume.cpp
    ${MU_ROOT_DIR}/source/SMD.cpp
    ${MU_ROOT_DIR}/source/SMD2BMD.cpp
    ${MU_ROOT_DIR}/source/GOBoid.cpp

    # ---- UI system (NEW) ----
    ${MU_ROOT_DIR}/source/UIControls.cpp
    ${MU_ROOT_DIR}/source/UIWindows.cpp
    ${MU_ROOT_DIR}/source/UIManager.cpp
    ${MU_ROOT_DIR}/source/UIDefaultBase.cpp
    ${MU_ROOT_DIR}/source/UIGuildInfo.cpp
    ${MU_ROOT_DIR}/source/UIGuildMaster.cpp
    ${MU_ROOT_DIR}/source/UIGateKeeper.cpp
    ${MU_ROOT_DIR}/source/UIGuardsMan.cpp
    ${MU_ROOT_DIR}/source/UIPopup.cpp
    ${MU_ROOT_DIR}/source/UIJewelHarmony.cpp
    ${MU_ROOT_DIR}/source/UISenatus.cpp
    ${MU_ROOT_DIR}/source/UIMapName.cpp
    ${MU_ROOT_DIR}/source/NewUISystem.cpp
    ${MU_ROOT_DIR}/source/NewUIManager.cpp
    ${MU_ROOT_DIR}/source/NewUICommon.cpp
    ${MU_ROOT_DIR}/source/NewUIButton.cpp
    ${MU_ROOT_DIR}/source/NewUIGroup.cpp
    ${MU_ROOT_DIR}/source/NewUIItemMng.cpp
    ${MU_ROOT_DIR}/source/NewUIRenderNumber.cpp
    ${MU_ROOT_DIR}/source/NewUISlideWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIScrollBar.cpp
    ${MU_ROOT_DIR}/source/NewUITextBox.cpp
    ${MU_ROOT_DIR}/source/NewUIMainFrameWindow.cpp
    ${MU_ROOT_DIR}/source/NewUICharacterInfoWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIMyInventory.cpp
    ${MU_ROOT_DIR}/source/NewUIInventoryCtrl.cpp
    ${MU_ROOT_DIR}/source/NewUIStorageInventory.cpp
    ${MU_ROOT_DIR}/source/NewUITrade.cpp
    ${MU_ROOT_DIR}/source/NewUINPCShop.cpp
    ${MU_ROOT_DIR}/source/NewUINPCQuest.cpp
    ${MU_ROOT_DIR}/source/NewUINPCDialogue.cpp
    ${MU_ROOT_DIR}/source/NewUICommonMessageBox.cpp
    ${MU_ROOT_DIR}/source/NewUICustomMessageBox.cpp
    ${MU_ROOT_DIR}/source/NewUIMessageBox.cpp
    ${MU_ROOT_DIR}/source/NewUIChatInputBox.cpp
    ${MU_ROOT_DIR}/source/NewUIChatLogWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIHotKey.cpp
    ${MU_ROOT_DIR}/source/NewUIOptionWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIWindowMenu.cpp
    ${MU_ROOT_DIR}/source/NewUICommandWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIFriendWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIHelpWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIHeroPositionInfo.cpp
    ${MU_ROOT_DIR}/source/NewUIMiniMap.cpp
    ${MU_ROOT_DIR}/source/NewUIPartyListWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIPartyInfoWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIPetInfoWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIGuildInfoWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIGuildMakeWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIGuardWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIGatemanWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIGateSwitchWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIMoveCommandWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIQuickCommandWindow.cpp
    ${MU_ROOT_DIR}/source/NewUINameWindow.cpp
    ${MU_ROOT_DIR}/source/NewUISetItemExplanation.cpp
    ${MU_ROOT_DIR}/source/NewUIItemExplanationWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIItemEnduranceInfo.cpp
    ${MU_ROOT_DIR}/source/NewUIMixInventory.cpp
    ${MU_ROOT_DIR}/source/NewUIMyShopInventory.cpp
    ${MU_ROOT_DIR}/source/NewUIPurchaseShopInventory.cpp
    ${MU_ROOT_DIR}/source/NewUISeigeWarfare.cpp
    ${MU_ROOT_DIR}/source/NewUISiegeWarBase.cpp
    ${MU_ROOT_DIR}/source/NewUISiegeWarCommander.cpp
    ${MU_ROOT_DIR}/source/NewUISiegeWarObserver.cpp
    ${MU_ROOT_DIR}/source/NewUISiegeWarSoldier.cpp
    ${MU_ROOT_DIR}/source/NewUICastleWindow.cpp
    ${MU_ROOT_DIR}/source/NewUICatapultWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIBloodCastleEnter.cpp
    ${MU_ROOT_DIR}/source/NewUIBloodCastleTime.cpp
    ${MU_ROOT_DIR}/source/NewUIEnterDevilSquare.cpp
    ${MU_ROOT_DIR}/source/NewUIBattleSoccerScore.cpp
    ${MU_ROOT_DIR}/source/NewUIDuelWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIDuelWatchWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIDuelWatchMainFrameWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIDuelWatchUserListWindow.cpp
    ${MU_ROOT_DIR}/source/NewUICryWolf.cpp
    ${MU_ROOT_DIR}/source/NewUIKanturuEvent.cpp
    ${MU_ROOT_DIR}/source/NewUICursedTempleEnter.cpp
    ${MU_ROOT_DIR}/source/NewUICursedTempleSystem.cpp
    ${MU_ROOT_DIR}/source/NewUICursedTempleResult.cpp
    ${MU_ROOT_DIR}/source/NewUIDoppelGangerFrame.cpp
    ${MU_ROOT_DIR}/source/NewUIDoppelGangerWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIEmpireGuardianNPC.cpp
    ${MU_ROOT_DIR}/source/NewUIEmpireGuardianTimer.cpp
    ${MU_ROOT_DIR}/source/NewUIGoldBowmanLena.cpp
    ${MU_ROOT_DIR}/source/NewUIGoldBowmanWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIGensRanking.cpp
    ${MU_ROOT_DIR}/source/NewUIRegistrationLuckyCoin.cpp
    ${MU_ROOT_DIR}/source/NewUIExchangeLuckyCoin.cpp
    ${MU_ROOT_DIR}/source/NewUILuckyItemWnd.cpp
    ${MU_ROOT_DIR}/source/NewUIMasterLevel.cpp
    ${MU_ROOT_DIR}/source/NewUIPCPoint.cpp
    ${MU_ROOT_DIR}/source/NewUIChaosCastleTime.cpp
    ${MU_ROOT_DIR}/source/NewUIQuestProgress.cpp
    ${MU_ROOT_DIR}/source/NewUIQuestProgressByEtc.cpp
    ${MU_ROOT_DIR}/source/NewUIMyQuestInfoWindow.cpp
    ${MU_ROOT_DIR}/source/NewUIUnitedMarketPlaceWindow.cpp
    ${MU_ROOT_DIR}/source/NewUI3DRenderMng.cpp
    ${MU_ROOT_DIR}/source/NewUIBuffWindow.cpp
    ${MU_ROOT_DIR}/source/NewGatemanWindow.cpp
    ${MU_ROOT_DIR}/source/StackInterface.cpp
    ${MU_ROOT_DIR}/source/MsgWin.cpp
    ${MU_ROOT_DIR}/source/SysMenuWin.cpp
    ${MU_ROOT_DIR}/source/CreditWin.cpp
    ${MU_ROOT_DIR}/source/OptionWin.cpp
    ${MU_ROOT_DIR}/source/Slider.cpp
    ${MU_ROOT_DIR}/source/SlideInfo.cpp
    ${MU_ROOT_DIR}/source/CGFxMainUi.cpp
    ${MU_ROOT_DIR}/source/CGFxProcess.cpp
    ${MU_ROOT_DIR}/source/CGFxInfoPopup.cpp
    ${MU_ROOT_DIR}/source/CGFxSample.cpp
    ${MU_ROOT_DIR}/source/CGFxFontConfigParser.cpp
    ${MU_ROOT_DIR}/source/BlackWin.cpp
    ${MU_ROOT_DIR}/source/Descriptions.cpp
    ${MU_ROOT_DIR}/source/CharInfoBalloon.cpp
    ${MU_ROOT_DIR}/source/CharInfoBalloonMng.cpp
    ${MU_ROOT_DIR}/source/CBTMessageBox.cpp

    # ---- Maps (NEW) ----
    ${MU_ROOT_DIR}/source/MapManager.cpp
    ${MU_ROOT_DIR}/source/GMHellas.cpp
    ${MU_ROOT_DIR}/source/GMBattleCastle.cpp
    ${MU_ROOT_DIR}/source/GMHuntingGround.cpp
    ${MU_ROOT_DIR}/source/GMAida.cpp
    ${MU_ROOT_DIR}/source/GMCrywolf1st.cpp
    ${MU_ROOT_DIR}/source/GMCryingWolf2nd.cpp
    ${MU_ROOT_DIR}/source/GM_kanturu_1st.cpp
    ${MU_ROOT_DIR}/source/GM_Kanturu_2nd.cpp
    ${MU_ROOT_DIR}/source/GM_Kanturu_3rd.cpp
    ${MU_ROOT_DIR}/source/GM_Raklion.cpp
    ${MU_ROOT_DIR}/source/GM3rdChangeUp.cpp
    ${MU_ROOT_DIR}/source/GMNewTown.cpp
    ${MU_ROOT_DIR}/source/GMSantaTown.cpp
    ${MU_ROOT_DIR}/source/GMSwampOfQuiet.cpp
    ${MU_ROOT_DIR}/source/GMDoppelGanger1.cpp
    ${MU_ROOT_DIR}/source/GMDoppelGanger2.cpp
    ${MU_ROOT_DIR}/source/GMDoppelGanger3.cpp
    ${MU_ROOT_DIR}/source/GMDoppelGanger4.cpp
    ${MU_ROOT_DIR}/source/GMEmpireGuardian1.cpp
    ${MU_ROOT_DIR}/source/GMEmpireGuardian2.cpp
    ${MU_ROOT_DIR}/source/GMEmpireGuardian3.cpp
    ${MU_ROOT_DIR}/source/GMEmpireGuardian4.cpp
    ${MU_ROOT_DIR}/source/GMDuelArena.cpp
    ${MU_ROOT_DIR}/source/GM_PK_Field.cpp
    ${MU_ROOT_DIR}/source/GMKarutan1.cpp
    ${MU_ROOT_DIR}/source/GMUnitedMarketPlace.cpp
    ${MU_ROOT_DIR}/source/w_CursedTemple.cpp
    ${MU_ROOT_DIR}/source/w_MapProcess.cpp
    ${MU_ROOT_DIR}/source/TestEventMap.cpp

    # ---- Protocol & Networking (NEW) ----
    ${MU_ROOT_DIR}/source/Protocol.cpp
    ${MU_ROOT_DIR}/source/ProtocolSend.cpp
    ${MU_ROOT_DIR}/source/NProtocol.cpp
    ${MU_ROOT_DIR}/source/CSMapServer.cpp
    ${MU_ROOT_DIR}/source/CSChaosCastle.cpp
    ${MU_ROOT_DIR}/source/SocketSystem.cpp

    # ---- Game systems (NEW) ----
    ${MU_ROOT_DIR}/source/_GlobalFunctions.cpp
    ${MU_ROOT_DIR}/source/Local.cpp
    ${MU_ROOT_DIR}/source/MultiLanguage.cpp
    ${MU_ROOT_DIR}/source/CSItemOption.cpp
    ${MU_ROOT_DIR}/source/CSQuest.cpp
    ${MU_ROOT_DIR}/source/CSPetSystem.cpp
    ${MU_ROOT_DIR}/source/CSParts.cpp
    ${MU_ROOT_DIR}/source/CSWaterTerrain.cpp
    # CSResourceManager.cpp excluded: RESOURCE_MANAGER not defined, depends on missing Public.h
    ${MU_ROOT_DIR}/source/CSEventMatch.cpp
    ${MU_ROOT_DIR}/source/CDirection.cpp
    ${MU_ROOT_DIR}/source/CKANTURUDirection.cpp
    ${MU_ROOT_DIR}/source/CMVP1stDirection.cpp
    ${MU_ROOT_DIR}/source/CameraMove.cpp
    ${MU_ROOT_DIR}/source/BoneManager.cpp
    ${MU_ROOT_DIR}/source/Camera3D.cpp
    ${MU_ROOT_DIR}/source/Widescreen.cpp
    ${MU_ROOT_DIR}/source/LoadingScene.cpp
    ${MU_ROOT_DIR}/source/MatchEvent.cpp
    ${MU_ROOT_DIR}/source/Event.cpp
    ${MU_ROOT_DIR}/source/MixMgr.cpp
    ${MU_ROOT_DIR}/source/PersonalShopTitleImp.cpp
    ${MU_ROOT_DIR}/source/GuildCache.cpp
    ${MU_ROOT_DIR}/source/GuildManager.cpp
    ${MU_ROOT_DIR}/source/GIPetManager.cpp
    ${MU_ROOT_DIR}/source/CComGem.cpp
    ${MU_ROOT_DIR}/source/npcBreeder.cpp
    ${MU_ROOT_DIR}/source/npcCatapult.cpp
    ${MU_ROOT_DIR}/source/npcGateSwitch.cpp
    ${MU_ROOT_DIR}/source/PartyManager.cpp
    ${MU_ROOT_DIR}/source/GameCensorship.cpp
    ${MU_ROOT_DIR}/source/GambleSystem.cpp
    ${MU_ROOT_DIR}/source/QuestMng.cpp
    ${MU_ROOT_DIR}/source/QuestInfo.cpp
    ${MU_ROOT_DIR}/source/SummonSystem.cpp
    ${MU_ROOT_DIR}/source/MonkSystem.cpp
    ${MU_ROOT_DIR}/source/DuelMgr.cpp
    ${MU_ROOT_DIR}/source/PortalMgr.cpp
    ${MU_ROOT_DIR}/source/GlobalPortalSystem.cpp
    ${MU_ROOT_DIR}/source/HelperManager.cpp
    ${MU_ROOT_DIR}/source/NewBloodCastleSystem.cpp
    ${MU_ROOT_DIR}/source/NewChaosCastleSystem.cpp
    ${MU_ROOT_DIR}/source/NewInventory.cpp
    ${MU_ROOT_DIR}/source/Preview.cpp
    ${MU_ROOT_DIR}/source/w_PetProcess.cpp
    ${MU_ROOT_DIR}/source/w_BasePet.cpp
    ${MU_ROOT_DIR}/source/w_PetActionStand.cpp
    ${MU_ROOT_DIR}/source/w_PetActionRound.cpp
    ${MU_ROOT_DIR}/source/w_PetActionDemon.cpp
    ${MU_ROOT_DIR}/source/w_PetActionUnicorn.cpp
    ${MU_ROOT_DIR}/source/w_PetActionCollecter.cpp
    ${MU_ROOT_DIR}/source/w_PetActionCollecter_Add.cpp
    ${MU_ROOT_DIR}/source/w_Buff.cpp
    ${MU_ROOT_DIR}/source/w_BuffScriptLoader.cpp
    ${MU_ROOT_DIR}/source/w_BuffStateSystem.cpp
    ${MU_ROOT_DIR}/source/w_BuffStateValueControl.cpp
    ${MU_ROOT_DIR}/source/w_BuffTimeControl.cpp
    ${MU_ROOT_DIR}/source/CharacterManager.cpp
    ${MU_ROOT_DIR}/source/SkillManager.cpp
    ${MU_ROOT_DIR}/source/SkillEffectMgr.cpp
    ${MU_ROOT_DIR}/source/SkillInfo.cpp
    ${MU_ROOT_DIR}/source/ItemAddOptioninfo.cpp
    ${MU_ROOT_DIR}/source/ItemInfo.cpp
    ${MU_ROOT_DIR}/source/ItemMove.cpp
    ${MU_ROOT_DIR}/source/MoveCommandData.cpp
    ${MU_ROOT_DIR}/source/MovereqInfo.cpp
    ${MU_ROOT_DIR}/source/FilterInfo.cpp
    ${MU_ROOT_DIR}/source/FilterNameInfo.cpp
    ${MU_ROOT_DIR}/source/DialogInfo.cpp
    ${MU_ROOT_DIR}/source/ExplanationInfo.cpp
    ${MU_ROOT_DIR}/source/CreditInfo.cpp
    ${MU_ROOT_DIR}/source/Observer.cpp
    ${MU_ROOT_DIR}/source/GCCertification.cpp
    ${MU_ROOT_DIR}/source/KeyGenerater.cpp
    ${MU_ROOT_DIR}/source/UsefulDef.cpp
    ${MU_ROOT_DIR}/source/ServerMsgWin.cpp
    ${MU_ROOT_DIR}/source/ChaosGenesis.cpp
    ${MU_ROOT_DIR}/source/TradeX.cpp
    ${MU_ROOT_DIR}/source/TradeXS.cpp
    ${MU_ROOT_DIR}/source/SideHair.cpp

    # ---- Custom systems (NEW) ----
    ${MU_ROOT_DIR}/source/DarkSpirit.cpp
    ${MU_ROOT_DIR}/source/DarkSpiritView.cpp
    ${MU_ROOT_DIR}/source/ElementPet.cpp
    ${MU_ROOT_DIR}/source/HelperView.cpp
    ${MU_ROOT_DIR}/source/CustomBow.cpp
    ${MU_ROOT_DIR}/source/CustomCape.cpp
    ${MU_ROOT_DIR}/source/CustomItemFloor.cpp
    ${MU_ROOT_DIR}/source/CustomItemForce.cpp
    ${MU_ROOT_DIR}/source/CustomWing.cpp
    ${MU_ROOT_DIR}/source/CustomJewel.cpp
    ${MU_ROOT_DIR}/source/CustomJewelStack.cpp
    ${MU_ROOT_DIR}/source/DisableExcellent.cpp
    ${MU_ROOT_DIR}/source/ElementSlots.cpp
    ${MU_ROOT_DIR}/source/Interfaces.cpp
    ${MU_ROOT_DIR}/source/ItemPosition.cpp
    ${MU_ROOT_DIR}/source/ItemSize.cpp
    ${MU_ROOT_DIR}/source/MessageColor.cpp
    ${MU_ROOT_DIR}/source/MonsterEffect.cpp
    ${MU_ROOT_DIR}/source/MonsterGlow.cpp
    ${MU_ROOT_DIR}/source/MonsterName.cpp
    ${MU_ROOT_DIR}/source/Monsters.cpp
    ${MU_ROOT_DIR}/source/RenderModel.cpp
    ${MU_ROOT_DIR}/source/ServerName.cpp
    ${MU_ROOT_DIR}/source/Visuals.cpp
    ${MU_ROOT_DIR}/source/CounterItem.cpp
    ${MU_ROOT_DIR}/source/CreateFont.cpp
    ${MU_ROOT_DIR}/source/Controller.cpp
    ${MU_ROOT_DIR}/source/Builder.cpp
    ${MU_ROOT_DIR}/source/WindowTray.cpp

    # ---- Lua 5.1.5 library source (compiled as C) ----
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/lapi.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/lauxlib.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/lbaselib.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/lcode.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/ldblib.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/ldebug.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/ldo.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/ldump.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/lfunc.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/lgc.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/linit.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/liolib.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/llex.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/lmathlib.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/lmem.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/loadlib.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/lobject.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/lopcodes.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/loslib.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/lparser.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/lstate.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/lstring.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/lstrlib.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/ltable.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/ltablib.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/ltm.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/lundump.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/lvm.c
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/lzio.c

    # ---- Lua scripting (NEW) ----
    ${MU_ROOT_DIR}/source/Lua.cpp
    ${MU_ROOT_DIR}/source/LuaBMD.cpp
    ${MU_ROOT_DIR}/source/LuaCharacter.cpp
    ${MU_ROOT_DIR}/source/LuaCloth.cpp
    ${MU_ROOT_DIR}/source/LuaDecrypt.cpp
    ${MU_ROOT_DIR}/source/LuaEffects.cpp
    ${MU_ROOT_DIR}/source/LuaEffectsNormal.cpp
    ${MU_ROOT_DIR}/source/LuaGlobal.cpp
    ${MU_ROOT_DIR}/source/LuaItemObject.cpp
    ${MU_ROOT_DIR}/source/LuaLoadImage.cpp
    ${MU_ROOT_DIR}/source/LuaObject.cpp
    ${MU_ROOT_DIR}/source/LuaOpenFolder.cpp
    ${MU_ROOT_DIR}/source/LuaReg.cpp
    ${MU_ROOT_DIR}/source/LuaSocket.cpp
    ${MU_ROOT_DIR}/source/LuaUser.cpp
    ${MU_ROOT_DIR}/source/LuaInterface.cpp

    # ---- Utilities (NEW) ----
    ${MU_ROOT_DIR}/source/Utilities/Log/DebugAngel.cpp
    # ErrorReport.cpp excluded — Android replacement in LegacyErrorReportCompat.cpp
    ${MU_ROOT_DIR}/source/Utilities/Log/muConsoleDebug.cpp
    ${MU_ROOT_DIR}/source/Utilities/Memory/MemoryLock.cpp
    ${MU_ROOT_DIR}/source/Time/CTimCheck.cpp

    # ---- GameShop (NEW) ----
    ${MU_ROOT_DIR}/source/GameShop/MsgBoxIGSBuyConfirm.cpp
    ${MU_ROOT_DIR}/source/GameShop/MsgBoxIGSBuyPackageItem.cpp
    ${MU_ROOT_DIR}/source/GameShop/MsgBoxIGSBuySelectItem.cpp
    ${MU_ROOT_DIR}/source/GameShop/MsgBoxIGSCommon.cpp
    ${MU_ROOT_DIR}/source/GameShop/MsgBoxIGSDeleteItemConfirm.cpp
    ${MU_ROOT_DIR}/source/GameShop/MsgBoxIGSGiftStorageItemInfo.cpp
    ${MU_ROOT_DIR}/source/GameShop/MsgBoxIGSSendGift.cpp
    ${MU_ROOT_DIR}/source/GameShop/MsgBoxIGSSendGiftConfirm.cpp
    ${MU_ROOT_DIR}/source/GameShop/MsgBoxIGSStorageItemInfo.cpp
    ${MU_ROOT_DIR}/source/GameShop/MsgBoxIGSUseBuffConfirm.cpp
    ${MU_ROOT_DIR}/source/GameShop/MsgBoxIGSUseItemConfirm.cpp
    ${MU_ROOT_DIR}/source/GameShop/NewUIInGameShop.cpp
    ${MU_ROOT_DIR}/source/GameShop/InGameShopSystem.cpp
    ${MU_ROOT_DIR}/source/GameShop/ShopListManager/BannerInfo.cpp
    ${MU_ROOT_DIR}/source/GameShop/ShopListManager/BannerInfoList.cpp
    ${MU_ROOT_DIR}/source/GameShop/ShopListManager/BannerListManager.cpp
    ${MU_ROOT_DIR}/source/GameShop/ShopListManager/ListManager.cpp
    ${MU_ROOT_DIR}/source/GameShop/ShopListManager/ShopCategory.cpp
    ${MU_ROOT_DIR}/source/GameShop/ShopListManager/ShopCategoryList.cpp
    ${MU_ROOT_DIR}/source/GameShop/ShopListManager/ShopList.cpp
    ${MU_ROOT_DIR}/source/GameShop/ShopListManager/ShopListManager.cpp
    ${MU_ROOT_DIR}/source/GameShop/ShopListManager/ShopPackage.cpp
    ${MU_ROOT_DIR}/source/GameShop/ShopListManager/ShopPackageList.cpp
    ${MU_ROOT_DIR}/source/GameShop/ShopListManager/ShopProduct.cpp
    ${MU_ROOT_DIR}/source/GameShop/ShopListManager/ShopProductList.cpp
    ${MU_ROOT_DIR}/source/GameShop/ShopListManager/FTPFileDownLoader.cpp
    ${MU_ROOT_DIR}/source/GameShop/ShopListManager/StringMethod.cpp
    ${MU_ROOT_DIR}/source/GameShop/ShopListManager/StringToken.cpp

    # ---- IJG JPEG 9e library source (compiled as C) ----
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jaricom.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jcapimin.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jcapistd.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jcarith.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jccoefct.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jccolor.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jcdctmgr.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jchuff.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jcinit.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jcmainct.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jcmarker.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jcmaster.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jcomapi.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jcparam.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jcprepct.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jcsample.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jctrans.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jdapimin.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jdapistd.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jdarith.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jdatadst.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jdatasrc.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jdcoefct.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jdcolor.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jddctmgr.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jdhuff.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jdinput.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jdmainct.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jdmarker.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jdmaster.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jdmerge.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jdpostct.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jdsample.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jdtrans.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jerror.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jfdctflt.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jfdctfst.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jfdctint.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jidctflt.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jidctfst.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jidctint.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jmemmgr.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jmemnobs.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jquant1.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jquant2.c
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src/jutils.c

    # ---- External libraries (NEW) ----
    ${MU_ROOT_DIR}/source/ExternalObject/Leaf/xstreambuf.cpp

    # ---- Missing game systems (NEW) ----
    ${MU_ROOT_DIR}/source/ChangeRingManager.cpp

    # ---- Platform layer (existing + NEW) ----
    ${MU_ROOT_DIR}/source/Platform/Season3BNamespaceCompat.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyObjectCompat.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyCharacterSceneCompat.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyCharacterSceneLinkCompat.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyTerrainCompat.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyMapManagerCompat.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyErrorReportCompat.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyAndroidClientStubs.cpp
    ${MU_ROOT_DIR}/source/Platform/GlobalBitmapAndroidCompat.cpp
    ${MU_ROOT_DIR}/source/Platform/ZzzTextureAndroidCompat.cpp
    ${MU_ROOT_DIR}/source/Platform/GameAssetPath.cpp
    ${MU_ROOT_DIR}/source/Platform/GameBootstrapTexture.cpp
    ${MU_ROOT_DIR}/source/Platform/GameClientConfig.cpp
    ${MU_ROOT_DIR}/source/Platform/GameConnectServerBootstrap.cpp
    ${MU_ROOT_DIR}/source/Platform/GameClientRuntimeConfig.cpp
    ${MU_ROOT_DIR}/source/Platform/GameConfigBootstrap.cpp
    ${MU_ROOT_DIR}/source/Platform/GameGlobalTextBootstrap.cpp
    ${MU_ROOT_DIR}/source/Platform/GameLanguageAssetBootstrap.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyClientRuntime.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyAndroidUiCompat.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyLoginUiRuntime.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyCharacterUiRuntime.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyLoginRenderCompat.cpp
    ${MU_ROOT_DIR}/source/Platform/LegacyLoginServerListRuntime.cpp
    ${MU_ROOT_DIR}/source/Platform/GameMouseInput.cpp
    ${MU_ROOT_DIR}/source/Platform/GamePacketCryptoBootstrap.cpp
    ${MU_ROOT_DIR}/source/Platform/GameServerBootstrap.cpp
    ${MU_ROOT_DIR}/source/Platform/AndroidTextRenderer.cpp
    ${MU_ROOT_DIR}/source/Platform/AndroidWin32Compat.cpp
    ${MU_ROOT_DIR}/source/Platform/RenderBackend.cpp
    ${MU_ROOT_DIR}/source/Platform/OpenGLESRenderBackend.cpp
    ${MU_ROOT_DIR}/source/Platform/AndroidEglWindow.cpp
    ${MU_ROOT_DIR}/source/Platform/AudioAndroidStub.cpp
    ${MU_ROOT_DIR}/source/Platform/AndroidNetworkPollCompat.cpp
    ${MU_ROOT_DIR}/source/Platform/AndroidGameInit.cpp
)

add_library(mu_android_legacy_login_ui_probe OBJECT
    ${MU_ROOT_DIR}/source/CriticalSection.cpp
    ${MU_ROOT_DIR}/source/Input.cpp
    ${MU_ROOT_DIR}/source/Win.cpp
    ${MU_ROOT_DIR}/source/Button.cpp
    ${MU_ROOT_DIR}/source/LoginMainWin.cpp
    ${MU_ROOT_DIR}/source/ServerSelWin.cpp
    ${MU_ROOT_DIR}/source/LoginWin.cpp
    ${MU_ROOT_DIR}/source/CharMakeWin.cpp
    ${MU_ROOT_DIR}/source/UIMng.cpp
)

target_include_directories(mu_android_bootstrap PRIVATE
    ${MU_ROOT_DIR}/source
    ${ANDROID_NDK}/sources/android/native_app_glue
)
# Dependencies/include added as SYSTEM to avoid its stale KHR/khrplatform.h
# from overriding the NDK's own version (which lacks __NDK_FPABI__).
target_include_directories(mu_android_bootstrap SYSTEM PRIVATE
    ${MU_ROOT_DIR}/Dependencies/include
    ${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src
    ${MU_ROOT_DIR}/Dependencies/jpeg-9e-src
)

target_compile_definitions(mu_android_bootstrap PRIVATE
    __NDK_FPABI__=
    MU_ANDROID_HAS_ZZZOPENGLUTIL_RUNTIME=1
    MU_ANDROID_HAS_ZZZSCENE_RUNTIME=1
    MU_ANDROID_HAS_ZZZCHARACTER_RUNTIME=1
    MU_ANDROID_HAS_ZZZINTERFACE_RUNTIME=1
    MU_ANDROID_HAS_UICONTROLS_RUNTIME=1
    MU_ANDROID_HAS_ZZZINFORMATION_RUNTIME=1
    MU_ANDROID_HAS_WIDESCREEN_RUNTIME=1
    MU_ANDROID_HAS_MULTILANGUAGE_RUNTIME=1
    MU_ANDROID_HAS_ZZZTEXTURE_RUNTIME=1
    MU_ANDROID_HAS_MAPMANAGER_RUNTIME=1
    MU_ANDROID_HAS_ZZZLODTERRAIN_RUNTIME=1
    MU_ANDROID_HAS_LEGACYOBJECT_RUNTIME=1
    PJH_NEW_SERVER_SELECT_MAP=1
    PBG_ADD_NEWCHAR_MONK=1
    PBG_ADD_NEWCHAR_MONK_SKILL=1
    PBG_ADD_NEWCHAR_MONK_ITEM=1
    ASG_ADD_KARUTAN_MONSTERS=1
    ASG_ADD_MAP_KARUTAN=1
    KJH_ADD_INGAMESHOP_UI_SYSTEM=1
    DUEL_SYSTEM=1
    WINDOWMODE=1
    PBG_ADD_SECRETBUFF=1
)

target_compile_options(mu_android_bootstrap PRIVATE
    -Wno-format-security
    -Wno-deprecated-declarations
    -Wno-writable-strings
    -Wno-tautological-pointer-compare
    -Wno-c++11-narrowing
    -ferror-limit=200
)

target_link_options(mu_android_bootstrap PRIVATE
    -Wl,--gc-sections
)

target_include_directories(mu_android_legacy_login_ui_probe PRIVATE
    ${MU_ROOT_DIR}/source
    ${ANDROID_NDK}/sources/android/native_app_glue
)

add_dependencies(mu_android_bootstrap mu_android_legacy_login_ui_probe)

find_library(android-lib android)
find_library(log-lib log)
find_library(egl-lib EGL)
find_library(glesv2-lib GLESv2)
find_library(dl-lib dl)

target_link_libraries(mu_android_bootstrap
    PRIVATE
    ${android-lib}
    ${dl-lib}
    ${log-lib}
    ${egl-lib}
    ${glesv2-lib}
)

# Lua 5.1.5 source triggers Android FORTIFY buffer-read checks (strchr in
# luaL_openlib). Disable FORTIFY for these C files only.
file(GLOB LUA_C_SOURCES "${MU_ROOT_DIR}/Dependencies/lua-5.1.5-src/*.c")
set_source_files_properties(${LUA_C_SOURCES} PROPERTIES
    COMPILE_FLAGS "-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0"
)
