#include "selgame.h"

#include "all.h"
#include "config.h"
#include "DiabloUI/diabloui.h"
#include "DiabloUI/text.h"
#include "DiabloUI/dialogs.h"
#include "DiabloUI/selok.h"
#include "cyrillic_mapper.h"

namespace dvl {

char selgame_Label[32];
char selgame_Ip[129] = "";
char selgame_Password[16] = "";
char selgame_Description[256];
bool selgame_enteringGame;
int selgame_selectedGame;
bool selgame_endMenu;
int *gdwPlayerId;
int gbDifficulty;
int heroLevel;

static _SNETPROGRAMDATA *m_client_info;
extern int provider;

constexpr UiArtTextButton SELGAME_OK = UiArtTextButton("OK", &UiFocusNavigationSelect, { PANEL_LEFT + 299, 427, 140, 35 }, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD);
static UiArtTextButton SELGAME_CANCEL = UiArtTextButton(toMappedBytes(L"Скасувати"), &UiFocusNavigationEsc, { PANEL_LEFT + 449, 427, 140, 35 }, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD);

UiArtText SELGAME_DESCRIPTION(selgame_Description, { PANEL_LEFT + 35, 256, 205, 192 });

namespace {

char title[32];
UiListItem SELDIFF_DIALOG_ITEMS[] = {
	{ toMappedBytes(L"Нормальна"), DIFF_NORMAL },
	{ toMappedBytes(L"Жахіття"), DIFF_NIGHTMARE },
	{ toMappedBytes(L"Пекло"), DIFF_HELL }
};
UiItem SELDIFF_DIALOG[] = {
	MAINMENU_BACKGROUND,
	MAINMENU_LOGO,
	UiArtText(title, { PANEL_LEFT + 24, 161, 590, 35 }, UIS_CENTER | UIS_BIG),
	UiArtText(selgame_Label, { PANEL_LEFT + 34, 211, 205, 33 }, UIS_CENTER | UIS_BIG), // DIFF
	SELGAME_DESCRIPTION,
	UiArtText(toMappedBytes(L"Оберіть складність"), { PANEL_LEFT + 299, 211, 295, 35 }, UIS_CENTER | UIS_BIG),
	UiList(SELDIFF_DIALOG_ITEMS, PANEL_LEFT + 300, 282, 295, 26, UIS_CENTER | UIS_MED | UIS_GOLD),
	SELGAME_OK,
	SELGAME_CANCEL,
};

constexpr UiArtText SELUDPGAME_TITLE = UiArtText(title, { PANEL_LEFT + 24, 161, 590, 35 }, UIS_CENTER | UIS_BIG);
static UiArtText SELUDPGAME_DESCRIPTION_LABEL = UiArtText(toMappedBytes(L"Опис:"), { PANEL_LEFT + 35, 211, 205, 192 }, UIS_MED);

UiListItem SELUDPGAME_DIALOG_ITEMS[] = {
	{ toMappedBytes(L"Створити гру"), 0 },
	{ toMappedBytes(L"Доєднатись до гри"), 1 },
};
UiItem SELUDPGAME_DIALOG[] = {
	MAINMENU_BACKGROUND,
	MAINMENU_LOGO,
	SELUDPGAME_TITLE,
	SELUDPGAME_DESCRIPTION_LABEL,
	SELGAME_DESCRIPTION,
	UiArtText(toMappedBytes(L"Обрати дію"), { PANEL_LEFT + 300, 211, 295, 33 }, UIS_CENTER | UIS_BIG),
	UiList(SELUDPGAME_DIALOG_ITEMS, PANEL_LEFT + 305, 255, 285, 26, UIS_CENTER | UIS_MED | UIS_GOLD),
	SELGAME_OK,
	SELGAME_CANCEL,
};

UiItem ENTERIP_DIALOG[] = {
	MAINMENU_BACKGROUND,
	MAINMENU_LOGO,
	SELUDPGAME_TITLE,
	SELUDPGAME_DESCRIPTION_LABEL,
	SELGAME_DESCRIPTION,
	UiArtText(toMappedBytes(L"Введіть адресу"), { PANEL_LEFT + 305, 211, 285, 33 }, UIS_CENTER | UIS_BIG),

	UiEdit(selgame_Ip, 128, { PANEL_LEFT + 305, 314, 285, 33 }, UIS_MED | UIS_GOLD),
	SELGAME_OK,
	SELGAME_CANCEL,
};

UiItem ENTERPASSWORD_DIALOG[] = {
	MAINMENU_BACKGROUND,
	MAINMENU_LOGO,
	SELUDPGAME_TITLE,
	SELUDPGAME_DESCRIPTION_LABEL,
	SELGAME_DESCRIPTION,
	UiArtText(toMappedBytes(L"Введіть пароль"), { PANEL_LEFT + 305, 211, 285, 33 }, UIS_CENTER | UIS_BIG),
	UiEdit(selgame_Password, 15, { PANEL_LEFT + 305, 314, 285, 33 }, UIS_MED | UIS_GOLD),
	SELGAME_OK,
	SELGAME_CANCEL,
};

} // namespace

void selgame_Free()
{
	ArtBackground.Unload();
}

void selgame_GameSelection_Init()
{
	selgame_enteringGame = false;
	selgame_selectedGame = 0;

	if (provider == SELCONN_LOOPBACK) {
		selgame_enteringGame = true;
		selgame_GameSelection_Select(0);
		return;
	}

	getIniValue("Phone Book", "Entry1", selgame_Ip, 128);
	strcpy(title, toMappedBytes(L"Клієнт-Сервер (TCP)"));
	UiInitList(0, 1, selgame_GameSelection_Focus, selgame_GameSelection_Select, selgame_GameSelection_Esc, SELUDPGAME_DIALOG, size(SELUDPGAME_DIALOG));
}

void selgame_GameSelection_Focus(int value)
{
	switch (value) {
	case 0:
		strcpy(selgame_Description, toMappedBytes(L"Створити гру з обраною складністю."));
		break;
	case 1:
		strcpy(selgame_Description, toMappedBytes(L"Введіть IP чи адресу сервера щоб доєднатись до вже створеної гри."));
		break;
	}
	WordWrapArtStr(selgame_Description, SELGAME_DESCRIPTION.rect.w);
}

/**
 * @brief Load the current hero level from save file
 * @param pInfo Hero info
 * @return always true
 */
BOOL UpdateHeroLevel(_uiheroinfo *pInfo)
{
	if (strcasecmp(pInfo->name, gszHero) == 0)
		heroLevel = pInfo->level;

	return true;
}

void selgame_GameSelection_Select(int value)
{
	selgame_enteringGame = true;
	selgame_selectedGame = value;

	gfnHeroInfo(UpdateHeroLevel);

	switch (value) {
	case 0:
		strcpy(title, toMappedBytes(L"Створити гру"));
		UiInitList(0, NUM_DIFFICULTIES - 1, selgame_Diff_Focus, selgame_Diff_Select, selgame_Diff_Esc, SELDIFF_DIALOG, size(SELDIFF_DIALOG));
		break;
	case 1:
		strcpy(title, toMappedBytes(L"Доєднатись до TCP гри"));
		UiInitList(0, 0, NULL, selgame_Password_Init, selgame_GameSelection_Init, ENTERIP_DIALOG, size(ENTERIP_DIALOG));
		break;
	}
}

void selgame_GameSelection_Esc()
{
	UiInitList(0, 0, NULL, NULL, NULL, NULL, 0);
	selgame_enteringGame = false;
	selgame_endMenu = true;
}

void selgame_Diff_Focus(int value)
{
	switch (value) {
	case DIFF_NORMAL:
		strcpy(selgame_Label, toMappedBytes(L"Нормальна"));
		strcpy(selgame_Description, toMappedBytes(L"Нормальна складність\nЦе те, де новий персонаж має почати свій похід на знищення Дьябло"));
		break;
	case DIFF_NIGHTMARE:
		strcpy(selgame_Label, toMappedBytes(L"Жахіття"));
		strcpy(selgame_Description, toMappedBytes(L"Жахлива складність\nМешканці лабіринту відїлися і становлять більшу загрозу. Рекомендовано лише для досвідчених персонажів."));
		break;
	case DIFF_HELL:
		strcpy(selgame_Label, toMappedBytes(L"Пекло"));
		strcpy(selgame_Description, toMappedBytes(L"Пекельна складність\nНайсильніші з підземних потвор повзають при воротах в пекло. Лише найдосвідченіші персонажі можуть дозволити собі сюди завітати."));
		break;
	}
	WordWrapArtStr(selgame_Description, SELGAME_DESCRIPTION.rect.w);
}

bool IsDifficultyAllowed(int value)
{
	if (value == 0 || (value == 1 && heroLevel >= 20) || (value == 2 && heroLevel >= 30)) {
		return true;
	}

	selgame_Free();

	if (value == 1)
		UiSelOkDialog(title, toMappedBytes(L"Ваш персонаж має досягти 20го рівня для того щоб ви могли увійти в мережеву гру на Жахливій складності."), false);
	if (value == 2)
		UiSelOkDialog(title, toMappedBytes(L"Ваш персонаж має досягти 30го рівня для того щоб ви могли увійти в мережеву гру на Пекельній складності."), false);

	LoadBackgroundArt("ui_art\\selgame.pcx");

	return false;
}

void selgame_Diff_Select(int value)
{
	if (!IsDifficultyAllowed(value)) {
		selgame_GameSelection_Select(0);
		return;
	}

	gbDifficulty = value;

	if (provider == SELCONN_LOOPBACK) {
		selgame_Password_Select(0);
		return;
	}

	selgame_Password_Init(0);
}

void selgame_Diff_Esc()
{
	if (provider == SELCONN_LOOPBACK) {
		selgame_GameSelection_Esc();
		return;
	}

	selgame_GameSelection_Init();
}

void selgame_Password_Init(int value)
{
	memset(&selgame_Password, 0, sizeof(selgame_Password));
	UiInitList(0, 0, NULL, selgame_Password_Select, selgame_Password_Esc, ENTERPASSWORD_DIALOG, size(ENTERPASSWORD_DIALOG));
}

void selgame_Password_Select(int value)
{
	if (selgame_selectedGame) {
		setIniValue("Phone Book", "Entry1", selgame_Ip);
		if (SNetJoinGame(selgame_selectedGame, selgame_Ip, selgame_Password, NULL, NULL, gdwPlayerId)) {
			if (!IsDifficultyAllowed(m_client_info->initdata->bDiff)) {
				selgame_GameSelection_Select(1);
				return;
			}

			UiInitList(0, 0, NULL, NULL, NULL, NULL, 0);
			selgame_endMenu = true;
		} else {
			selgame_Free();
			UiSelOkDialog(toMappedBytes(L"Мережева гра"), SDL_GetError(), false);
			LoadBackgroundArt("ui_art\\selgame.pcx");
			selgame_Password_Init(selgame_selectedGame);
		}
		return;
	}

	_gamedata *info = m_client_info->initdata;
	info->bDiff = gbDifficulty;

	if (SNetCreateGame(NULL, selgame_Password, NULL, 0, (char *)info, sizeof(_gamedata), MAX_PLRS, NULL, NULL, gdwPlayerId)) {
		UiInitList(0, 0, NULL, NULL, NULL, NULL, 0);
		selgame_endMenu = true;
	} else {
		selgame_Free();
		UiSelOkDialog(toMappedBytes(L"Мережева гра"), SDL_GetError(), false);
		LoadBackgroundArt("ui_art\\selgame.pcx");
		selgame_Password_Init(0);
	}
}

void selgame_Password_Esc()
{
	selgame_GameSelection_Select(selgame_selectedGame);
}

int UiSelectGame(int a1, _SNETPROGRAMDATA *client_info, _SNETPLAYERDATA *user_info, _SNETUIDATA *ui_info,
    _SNETVERSIONDATA *file_info, int *playerId)
{
	gdwPlayerId = playerId;
	m_client_info = client_info;
	LoadBackgroundArt("ui_art\\selgame.pcx");
	selgame_GameSelection_Init();

	selgame_endMenu = false;
	while (!selgame_endMenu) {
		UiClearScreen();
		UiPollAndRender();
	}
	selgame_Free();

	return selgame_enteringGame;
}
} // namespace dvl
