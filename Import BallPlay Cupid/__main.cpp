// Lic:
// BallPlay++
// BallPlay Cupid Puzzle Importer
// 
// 
// 
// (c) Jeroen P. Broks, 2022
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// Please note that some references to data like pictures or audio, do not automatically
// fall under this licenses. Mostly this is noted in the respective files.
// 
// Version: 22.10.19
// EndLic
#include <QuickString.hpp>
#include <QuickInput.hpp>
#include <QCol.hpp>
#include <GINIE.hpp>

#include <FileList.hpp>
#include <lua.hpp>
#include <SuperTed_Save.hpp>
#include <jcr6_core.hpp>
#include <QuickStream.hpp>

#include "../../Game/BallPlay++/headers/dir_obj.h"

using namespace std;
using namespace TrickyUnits;  
using namespace SuperTed;
using namespace jcr6;

#pragma region Errors
void Error(string msg, bool fatal = false,bool show=false) {
	static int count{ 0 };
	if (show) {
		switch (count) {
		case 0: return;
		case 1:
			QCol->Yellow("There was "); 
			QCol->Red("1 ");
			QCol->Yellow("error!");
			QCol->Reset();
			return;
		default:
			QCol->Yellow("There were ");
			QCol->Red(to_string(count));
			QCol->Yellow(" errors!");
			QCol->Reset();
			return;
		}
	}
	QCol->Red("Error #"); printf("%03d ", ++count);
	QCol->Yellow(msg);
	QCol->Reset();
	cout << endl;
	if (fatal) exit(10);
}
#pragma endregion

#pragma region Config
GINIE Config;
GINIE PID;
string Ask(string Cat, string Key, string Question, string DefaultValue = "") {
	while (!Config.Value(Cat, Key).size()) {
		if (DefaultValue.size()) QCol->Magenta("[" + DefaultValue + "] ");
		QCol->Yellow(Question + " ");
		QCol->Color(qColor::Cyan,qColor::Black);
		Config.Value(Cat, Key, ReadLine());
		if (!Config.Value(Cat, Key).size()) Config.Value(Cat, Key, DefaultValue);
	}
	return Config.Value(Cat, Key);
}
#pragma endregion

#pragma region DirectoryVars
string OriDir;
string TarDir;
string PuzzleDir;
#pragma endregion

#pragma region TextureScan
class TTex {
private:
	static int teller;
public:
	string OriFile;
	string TarFile;
	int index{ 0 };
	TTex() {}
	TTex(string O, string T) {
		OriFile = O;
		TarFile = T;
		index = teller++;
		cout << "Texture '" << O << "' set up as '" << T << "' (index: " << index << ")\n";
	}
};
int TTex::teller{ 2000 };
map<string, TTex> TexReg;
void ScanTex(JT_Create* Out) {
	//E:\Projects\Applications\Love2D\Games\BallplayCupid\Stuff\Tricky Assets CCBYNCSA\GFX\Game\Puzzle
	string
		OTexDir[3]{
			OriDir + "Stuff/Tricky Assets CCBYNCSA/GFX/Game/Puzzle/",
			OriDir + "Stuff/Man of Steel/gfx/game/puzzle/",
			OriDir + "Stuff/ImageAfter/gfx/game/puzzle/"
		},
		JTexDir{ "Packages/BallPlay Cupid/Textures/" };
	map<string,string>
		D{  };
	for (auto OTD : OTexDir) {
		auto gD{ GetTree(OTD) };
		for (auto F : gD) D[F]=OTD + F;
	}
	for (auto fI : D) {
		auto f{ fI.second };
		auto tagf{ "pz_" + StripExt(TReplace(Lower(fI.first),"/","_")) };
		tagf = TReplace(tagf, ' ', '_');
		if (prefixed(tagf, "pz_floors_")) tagf = "pz_floor" + right(tagf, tagf.size() - 9); // Dirty, but should work!
		if (prefixed(tagf, "pz_walls_")) tagf = "pz_wall" + right(tagf, tagf.size() - 8); // Dirty, but should work!
		TexReg[tagf] = TTex( f, JTexDir + tagf + ".png");
		if (!FileExists( f)) Error( f + " not found!");
		cout << "Adding as: " << tagf<<" ... ";
		Out->AddFile(f, "Textures/" + tagf + ".png");
		cout << JCR_Error << endl;
	}
}

#pragma region SuperTed
map<int, Teddy> MapMap;
#pragma endregion

#pragma region LuaAPIs
lua_State* LS;
int LuaPaniek(lua_State* L) { 
	string Trace{};
	Error("Lua Error!"); 
	cout << lua_gettop(L) << "\n";
	for (int i = 1; i <= lua_gettop(L); i++) {
		cout << "Arg #" << i << "\t";
		switch (lua_type(L, i)) {
		case LUA_TSTRING:
			cout << "String \"" << luaL_checkstring(L, i);
			Trace += luaL_checkstring(L, i); Trace += "\n";
			break;
		case LUA_TNUMBER:
			cout << "Number " << luaL_checknumber(L, i);
		case LUA_TFUNCTION:
			cout << "Function";
		default:
			cout << "Unknown: " << lua_type(L, i);
			break;
		}
		cout << "\n";
	}
	Error("", false, true);
	exit(11);
	return 0;
}
int LA_Hello(lua_State* L) { cout << "Testing Lua -- Ok!\n"; return 0; }
int LA_TexLayModify(lua_State* L) {
	auto
		Lay{ luaL_checkstring(L,2) };
	string
		Tex{ luaL_checkstring(L,5) };
	auto
		ln{ luaL_checkinteger(L,1) },
		x{ luaL_checkinteger(L,3) },
		y{ luaL_checkinteger(L,4) };
	Tex = TReplace(Tex, ' ', '_');
	if (!TexReg.count(Tex)) { 
		char mERR[400]; sprintf_s(mERR,"No texture registered as '%s' on coordinates (%02d,%02d) of layer: '%s'", Tex.c_str(), (int)x, (int)y, Lay);
		Error(mERR);
		for (auto TexI : TexReg) cout << TexI.first << " = " << TexI.second.index << endl; exit(40); // debug only!
		return 0; 
	}
	if (TexReg[Tex].index > 65535) Error("Tex number too high: " + to_string(TexReg[Tex].index)); 
	MapMap[ln]->Rooms["PUZZLE"]->LayVal(Upper(Lay), x, y, TexReg[Tex].index);
	MapMap[ln]->Tex(TexReg[Tex].index)->TexFile = TexReg[Tex].TarFile;
	MapMap[ln]->Tex(TexReg[Tex].index)->Type = TeddyTexType::BottomCenter;
	return 0;
}
int LA_DirLayModify(lua_State* L) {
	auto
		ln{ luaL_checkinteger(L,1) },
		x{ luaL_checkinteger(L,2) },
		y{ luaL_checkinteger(L,3) },
		value{ luaL_checkinteger(L,4) };
	MapMap[ln]->Rooms["PUZZLE"]->LayVal("DIRECTIONS", x, y, value);
}

bool Script(string File) {
	if (!FileExists(File)) { Error("Script '" + File + "' has not been found!"); return false; }
	auto S{ LoadString(File) };
	cout << "Compiling: " << File << endl;
	luaL_loadstring(LS, S.c_str());
	lua_call(LS, 0, 0);
	return true;
}
int LA_Script(lua_State* L) {
	lua_pushboolean(L, Script(luaL_checkstring(L, 1)));
	return 1;
}
int LA_Puzzle(lua_State* L) {
	auto pnum{ (int)luaL_checkinteger(L,1) };
	char fname[400];
	sprintf_s(fname,"%s/Stuff/Tricky Script/Script/game.lll/Puzzles/Pz%02d.lua",OriDir.c_str(),pnum);
	if (!FileExists(fname)) { Error("Puzzle Data Script '" + string(fname) + "' not found!"); lua_pushboolean(L, false); return 1; }
	lua_pushboolean(L, true);
	lua_pushstring(L, LoadString(fname).c_str());
	return 2; 
}

int LA_Name(lua_State* L) {
	auto pnumb{ luaL_checkinteger(L,1) };
	auto pname{ luaL_checkstring(L,2) };
	MapMap[pnumb]->Data["Title"] = pname;
	QCol->Doing("Puzzle #" + to_string(pnumb), pname); QCol->Reset();
	return 0;
}

int LA_BreakBlock(lua_State* L) {
	static map<string, long long> TexN;
	auto
		m{ luaL_checkinteger(L,1) },
		x{ luaL_checkinteger(L,2) },
		y{ luaL_checkinteger(L,3) },
		r{ luaL_checkinteger(L,4) },
		g{ luaL_checkinteger(L,5) },
		b{ luaL_checkinteger(L,6) },
		tel{ (lua_Integer)4000 };
	char
		tag[20]; sprintf_s(tag, "bb%02x%02x%02x", (int)r, (int)g, (int)b);
	if (!TexN.count(tag)) {
		for (auto kv : TexN) tel = std::max(tel, kv.second + 1);
		TexN[tag] = tel;
	}
	auto Tex{ MapMap[m]->Tex(TexN[tag]) };
	Tex->TexFile = "PACKAGES/BALLPLAY CUPID/TEXTURES/pz_obstacles_breakblock.png";
	Tex->r = r;
	Tex->g = g;
	Tex->b = b;
	Tex->alpha = 255;
	cout << "Creating texture tag '" << tag << "' assigned to tex position #" << tel << ".\n";
	cout << "Placing: " << TexN[tag] << "\n";
	MapMap[m]->Rooms["PUZZLE"]->LayVal("BREAK", x, y, TexN[tag]);
	return 0;
}

int LA_MainData(lua_State* L) {
	auto
		pnum{ luaL_checkinteger(L,1) };
	auto
		pkey{ luaL_checkstring(L,2) },
		pval{ luaL_checkstring(L,3) };
	MapMap[pnum]->Data[pkey] = pval;
	return 0;
}

int LA_NonFatal(lua_State* L) {
	Error(luaL_checkstring(L, 1));
	return 0;
}

int LA_Object(lua_State* L) {
	auto
		pnum{ luaL_checkinteger(L,1) },
		type{ luaL_checkinteger(L,2) },
		xpos{ luaL_checkinteger(L,3) },
		ypos{ luaL_checkinteger(L,4) },
		rcol{ luaL_optinteger(L,5,255) },
		gcol{ luaL_optinteger(L,6,255) },
		bcol{ luaL_optinteger(L,7,255) },
		alph{ luaL_optinteger(L,8,255) };
	//ball{ luaL_optinteger(L,10,1) };
	auto
		sdir{ luaL_optstring(L,9,"South") };
	auto
		o{ MapMap[pnum]->Rooms["PUZZLE"]->AddObject(xpos,ypos,type) };
	o->kind = type;
	o->Data["Red"] = to_string(rcol);
	o->Data["Green"] = to_string(gcol);
	o->Data["Blue"] = to_string(bcol);
	o->Data["Alpha"] = to_string(alph);
	o->Data["Direction"] = sdir;
	cout << "Object (" << type << ") created at (" << xpos << "," << ypos << ") -> " << sdir << " (Color #";
	printf("%02x%02x%02x", rcol, gcol, bcol);
	cout << "; Alpha: " << alph << endl;
	return 0;
}

int LA_Laser(lua_State* L) {
	using namespace BallPlay;
	auto
		p{ luaL_checkinteger(L,1) },
		x{ luaL_checkinteger(L,2) },
		y{ luaL_checkinteger(L,3) },
		T{ luaL_checkinteger(L,4) };

	//auto
	//	D{ luaL_checkstring(L,5) },
	//	C{ luaL_checkstring(L,6) };
	auto
		R{ MapMap[p]->Rooms["PUZZLE"] };
	auto
		o{ R->AddObject(x,y,T) };
	byte r{ 0 }, g{ 0 }, b{ 0 };
	string dir{ "South" };
	switch (T) {

		// Red
	case LaserRedNorth:
		r = 255; g = 0; b = 0;
		dir = "North";
		break;
	case LaserRedSouth:
		r = 255; g = 0; b = 0;
		dir = "South";
	case LaserRedEast:
		r = 255; g = 0; b = 0;
		dir = "East";
		break;
	case LaserRedWest:
		r = 255; g = 0; b = 0;
		dir = "West";
		break;

		// Ember
	case LaserEmberNorth:
		r = 255; g = 180; b = 0;
		dir = "North";
		break;
	case LaserEmberSouth:
		r = 255; g = 180; b = 0;
		dir = "South";
	case LaserEmberEast:
		r = 255; g = 180; b = 0;
		dir = "East";
		break;
	case LaserEmberWest:
		r = 255; g = 180; b = 0;
		dir = "West";
		break;

		// Green
	case LaserGreenNorth:
		r = 0; g = 255; b = 0;
		dir = "North";
		break;
	case LaserGreenSouth:
		r = 0; g = 255; b = 0;
		dir = "South";
	case LaserGreenEast:
		r = 0; g = 255; b = 0;
		dir = "East";
		break;
	case LaserGreenWest:
		r = 0; g = 255; b = 0;
		dir = "West";
		break;

		// Blue
	case LaserBlueNorth:
		r = 0; g = 0; b = 0xff;
		dir = "North";
		break;
	case LaserBlueSouth:
		r = 0; g = 0; b = 0xff;
		dir = "South";
	case LaserBlueEast:
		r = 0; g = 0; b = 0xff;
		dir = "East";
		break;
	case LaserBlueWest:
		r = 0; g = 0; b = 0xff;
		dir = "West";
		break;
	default:
		Error("Unknown laser type!");
		break;

	}
	o->Data["Red"] = to_string(r);
	o->Data["Green"] = to_string(g);
	o->Data["Blue"] = to_string(b);
	o->Data["Direction"] = dir;
	R->LayVal("WALL", x, y, T);
	R->LayVal("DIRECTIONS", x, y, T); // Prevents placing plates and stuff on the lasers.
	return 0;
}

int LA_Death(lua_State* L) {
	auto
		n{ (int)luaL_checkinteger(L, 1) },
		x{ (int)luaL_checkinteger(L, 2) },
		y{ (int)luaL_checkinteger(L, 3) };
	printf("DEATH %d: (%02d,%02d)\n", n, x, y);
	//*
	MapMap[n]->Rooms["PUZZLE"]->Layers["DEATH"]->Field->Value(x, y, 1);
	//*/
	return 0;
}
#pragma endregion

#pragma region Background_Fractals
void Fractal(JT_Create* J, int pn) {
	static bool Done[10]{ false,false,false,false,false,false,false,false,false,false };
	auto fracn{ pn % 10 };
	auto fracf{ "Stuff/Fractint/gfx/game/fractals/f-%d.png" };
	auto fract{ "Background/f-%d.png" };
	char fracg[300]; sprintf_s(fracg, fracf, fracn);
	char frace[300]; sprintf_s(frace, fract, fracn);
	MapMap[pn]->Data["Background"] = string("Packages/BallPlay Cupid/") + frace;
	if (!Done[fracn]) {
		cout << "Importing: " << fracg << endl;
		if (!FileExists(OriDir + fracg)) Error("Fractal '" + OriDir + fracg + "' not found!");
		J->AddFile(OriDir + fracg, frace);
		Done[fracn] = true;
	}
}
#pragma endregion

#pragma region Tools
void Tools(JT_Create* Out) {
	using namespace BallPlay;
	string
		odir{ "Stuff/Tricky Assets CCBYNCSA/GFX/Game/Puzzle/Obstacles/" },
		tdir{ "Tools/" },
		files[4]{ "Plate1","Plate2","Barrier","Trashcan" };
	int
		codes[4]{ userplate1,userplate2,1,0 },
		r[4] = { 255,255,255,255 },
		g[4] = { 0,0,255,255 },
		b[4] = { 0,0,255,255 };

	for (int i = 0; i < 4; i++) {
		string tgt = tdir + files[i]+".png"; if (i == 3) tgt = "Tools/Remove.png";
		string src = OriDir + odir + files[i] + ".png";
		cout << "Adding tool: " << files[i]<< "\n";
		if (!FileExists(src)) {
			Error("Source tool file '" + src + "' not found!");
		} else {
			Out->AddFile(src, tgt);
		}
		for (auto j : MapMap) {
			if (codes[i]) {
				j.second->Tex(codes[i])->TexFile = "Packages/BallPlay Cupid/" + tgt;
				j.second->Tex(codes[i])->r = r[i];
				j.second->Tex(codes[i])->g = g[i];
				j.second->Tex(codes[i])->b = b[i];
			}
		}
	}
	Out->AddFile(OriDir + "Stuff/Tricky Assets CCBYNCSA/GFX/Game/Puzzle/Obstacles/Exit.png","Dir/Exit.png");
	Out->AddFile(OriDir + "Stuff/OCAL/GFX/Game/Puzzle/Obstacles/Hut.png", "Dir/Hut.png");
	Out->AddFile(OriDir + "Stuff/Tricky Assets CCBYNCSA/GFX/Game/Puzzle/Obstacles/LaserUp.png", "Lasers/North.png");
	Out->AddFile(OriDir + "Stuff/Tricky Assets CCBYNCSA/GFX/Game/Puzzle/Obstacles/LaserDown.png", "Lasers/South.png");
	Out->AddFile(OriDir + "Stuff/Tricky Assets CCBYNCSA/GFX/Game/Puzzle/Obstacles/LaserLeft.png", "Lasers/West.png");
	Out->AddFile(OriDir + "Stuff/Tricky Assets CCBYNCSA/GFX/Game/Puzzle/Obstacles/LaserRight.png", "Lasers/East.png");
	for (auto j : MapMap) {
		j.second->Tex(irreplacableplate1)->TexFile = j.second->Tex(userplate1)->TexFile;
		j.second->Tex(irreplacableplate1)->r = 0;
		j.second->Tex(irreplacableplate2)->TexFile = j.second->Tex(userplate2)->TexFile;
		j.second->Tex(irreplacableplate2)->r = 0;

		j.second->Tex(levelplate1)->TexFile = j.second->Tex(userplate1)->TexFile;
		j.second->Tex(levelplate1)->b = 0;
		j.second->Tex(levelplate2)->TexFile = j.second->Tex(userplate2)->TexFile;
		j.second->Tex(levelplate2)->b = 0;

		// Exits
		j.second->Tex(Exit)->TexFile = "Packages/BallPlay Cupid/Dir/Exit.png";
		j.second->Tex(GreenExit)->TexFile = "Packages/BallPlay Cupid/Dir/Exit.png";
		j.second->Tex(GreenExit)->r = 0;
		j.second->Tex(GreenExit)->b = 0;
		j.second->Tex(RedExit)->TexFile = "Packages/BallPlay Cupid/Dir/Exit.png";
		j.second->Tex(RedExit)->g = 0;
		j.second->Tex(RedExit)->b = 0;
		j.second->Tex(BlueExit)->TexFile = "Packages/BallPlay Cupid/Dir/Exit.png";
		j.second->Tex(BlueExit)->r = 0;
		j.second->Tex(BlueExit)->g = 0;
		j.second->Tex(EmberExit)->TexFile = "Packages/BallPlay Cupid/Dir/Exit.png";
		j.second->Tex(EmberExit)->g = 180;
		j.second->Tex(EmberExit)->b = 0;

		// Lasers
		j.second->Tex(LaserRedNorth)->TexFile = "Packages/BallPlay Cupid/Lasers/North.png";
		j.second->Tex(LaserRedNorth)->g = 0;
		j.second->Tex(LaserRedNorth)->b = 0;
		j.second->Tex(LaserRedSouth)->TexFile = "Packages/BallPlay Cupid/Lasers/South.png";
		j.second->Tex(LaserRedSouth)->g = 0;
		j.second->Tex(LaserRedSouth)->b = 0;
		j.second->Tex(LaserRedWest)->TexFile = "Packages/BallPlay Cupid/Lasers/West.png";
		j.second->Tex(LaserRedWest)->g = 0;
		j.second->Tex(LaserRedWest)->b = 0;
		j.second->Tex(LaserRedEast)->TexFile = "Packages/BallPlay Cupid/Lasers/East.png";
		j.second->Tex(LaserRedEast)->g = 0;
		j.second->Tex(LaserRedEast)->b = 0;

		j.second->Tex(LaserBlueNorth)->TexFile = "Packages/BallPlay Cupid/Lasers/North.png";
		j.second->Tex(LaserBlueNorth)->g = 0;
		j.second->Tex(LaserBlueNorth)->r = 0;
		j.second->Tex(LaserBlueSouth)->TexFile = "Packages/BallPlay Cupid/Lasers/South.png";
		j.second->Tex(LaserBlueSouth)->g = 0;
		j.second->Tex(LaserBlueSouth)->r = 0;
		j.second->Tex(LaserBlueWest)->TexFile = "Packages/BallPlay Cupid/Lasers/West.png";
		j.second->Tex(LaserBlueWest)->g = 0;
		j.second->Tex(LaserBlueWest)->r = 0;
		j.second->Tex(LaserBlueEast)->TexFile = "Packages/BallPlay Cupid/Lasers/East.png";
		j.second->Tex(LaserBlueEast)->g = 0;
		j.second->Tex(LaserBlueEast)->r = 0;

		j.second->Tex(LaserEmberNorth)->TexFile = "Packages/BallPlay Cupid/Lasers/North.png";
		j.second->Tex(LaserEmberNorth)->g = 0xb4;
		j.second->Tex(LaserEmberNorth)->b = 0x00;
		j.second->Tex(LaserEmberSouth)->TexFile = "Packages/BallPlay Cupid/Lasers/South.png";
		j.second->Tex(LaserEmberSouth)->g = 0xb4;
		j.second->Tex(LaserEmberSouth)->b = 0x00;
		j.second->Tex(LaserEmberWest)->TexFile = "Packages/BallPlay Cupid/Lasers/West.png";
		j.second->Tex(LaserEmberWest)->g = 0xb4;
		j.second->Tex(LaserEmberWest)->b = 0x00;
		j.second->Tex(LaserEmberEast)->TexFile = "Packages/BallPlay Cupid/Lasers/East.png";
		j.second->Tex(LaserEmberEast)->g = 0xb4;
		j.second->Tex(LaserEmberEast)->b = 0x00;

		j.second->Tex(LaserGreenNorth)->TexFile = "Packages/BallPlay Cupid/Lasers/North.png";
		j.second->Tex(LaserGreenNorth)->b = 0;
		j.second->Tex(LaserGreenNorth)->r = 0;
		j.second->Tex(LaserGreenSouth)->TexFile = "Packages/BallPlay Cupid/Lasers/South.png";
		j.second->Tex(LaserGreenSouth)->b = 0;
		j.second->Tex(LaserGreenSouth)->r = 0;
		j.second->Tex(LaserGreenWest)->TexFile = "Packages/BallPlay Cupid/Lasers/West.png";
		j.second->Tex(LaserGreenWest)->b = 0;
		j.second->Tex(LaserGreenWest)->r = 0;
		j.second->Tex(LaserGreenEast)->TexFile = "Packages/BallPlay Cupid/Lasers/East.png";
		j.second->Tex(LaserGreenEast)->b = 0;
		j.second->Tex(LaserGreenEast)->r = 0;

		j.second->Tex(LaserPlateBlue)->TexFile = "Packages/BallPlay Cupid/Textures/pz_obstacles_laserplate.png";
		j.second->Tex(LaserPlateBlue)->r = 0;
		j.second->Tex(LaserPlateBlue)->g = 0;
		j.second->Tex(LaserPlateBlue)->b = 255;

		j.second->Tex(LaserPlateRed)->TexFile = "Packages/BallPlay Cupid/Textures/pz_obstacles_laserplate.png";
		j.second->Tex(LaserPlateRed)->r = 255;
		j.second->Tex(LaserPlateRed)->g = 0;
		j.second->Tex(LaserPlateRed)->b = 0;

		j.second->Tex(LaserPlateGreen)->TexFile = "Packages/BallPlay Cupid/Textures/pz_obstacles_laserplate.png";
		j.second->Tex(LaserPlateGreen)->r = 0;
		j.second->Tex(LaserPlateGreen)->g = 255;
		j.second->Tex(LaserPlateGreen)->b = 0;

		j.second->Tex(LaserPlateEmber)->TexFile = "Packages/BallPlay Cupid/Textures/pz_obstacles_laserplate.png";
		j.second->Tex(LaserPlateEmber)->r = 255;
		j.second->Tex(LaserPlateEmber)->g = 180;
		j.second->Tex(LaserPlateEmber)->b = 0;

		// Arrows
		j.second->Tex(arrownorth)->TexFile = "Packages/BallPlay Cupid/Textures/pz_obstacles_arrow_up.png";
		j.second->Tex(arrownorth)->r = 0;
		j.second->Tex(arrownorth)->g = 180;
		j.second->Tex(arrownorth)->b = 255;
		j.second->Tex(arrowsouth)->TexFile = "Packages/BallPlay Cupid/Textures/pz_obstacles_arrow_down.png";
		j.second->Tex(arrowsouth)->r = 0;
		j.second->Tex(arrowsouth)->g = 180;
		j.second->Tex(arrowsouth)->b = 255;
		j.second->Tex(arrowwest)->TexFile = "Packages/BallPlay Cupid/Textures/pz_obstacles_arrow_left.png";
		j.second->Tex(arrowwest)->r = 0;
		j.second->Tex(arrowwest)->g = 180;
		j.second->Tex(arrowwest)->b = 255;
		j.second->Tex(arroweast)->TexFile = "Packages/BallPlay Cupid/Textures/pz_obstacles_arrow_right.png";
		j.second->Tex(arroweast)->r = 0;
		j.second->Tex(arroweast)->g = 180;
		j.second->Tex(arroweast)->b = 255;



		j.second->Tex(GirlHome)->TexFile = "Packages/BallPlay Cupid/Dir/Hut.png";
		j.second->Tex(dot)->TexFile = "PACKAGES/BALLPLAY CUPID/TEXTURES/pz_obstacles_dot.png";

	}
		

}
#pragma endregion

#pragma Objects
void Objects(JT_Create* Out) {
	string d{ OriDir + "Stuff/Tricky Assets CCBYNCSA/GFX/Game/Puzzle/Objects/" };
	Out->AddFile(d + "Ball.png", "Objects/Ball.png");
	Out->AddFile(d + "Ghost.png", "Objects/Ghost.png");
	Out->AddFile(d + "Robot.png", "Objects/Droid.png");
}

int main(int cnt, char** args) {
	cout << StripAll(args[0]) << " - Coded by Jeroen P. Broks!\n";
	cout << "Compiled: " << __DATE__"\n";
	auto ConfigFile{ ExtractDir(args[0]) + "/Import BallPlay Cupid.ini" };
	cout << "Reading config: " << ConfigFile << endl;
	Config.FromFile(ConfigFile, true);
	Config.AutoSave = ConfigFile;
	OriDir = Ask("Dir", "Original", "Where is the original version of BallPlay Cupid stored? "); if (!suffixed(OriDir, "/")) OriDir += "/";
	TarDir = Ask("Dir", "Target", "Where to put the target packages? "); if (!suffixed(TarDir, "/")) TarDir += "/";
	cout << "Initializing JCR6\n";
	init_JCR6();
	cout << "Initializing Lua State\n";
	LS = luaL_newstate();
	if (LS == NULL) {
		Error("Cannot create state : not enough memory");
		return 0;
	}
	luaL_openlibs(LS);
	lua_atpanic(LS, LuaPaniek);
	lua_register(LS, "BPC_Test", LA_Hello);
	lua_register(LS, "TexLayModify", LA_TexLayModify);
	lua_register(LS, "DirLayModify", LA_DirLayModify);
	lua_register(LS, "Script", LA_Script);
	lua_register(LS, "GetPuzzle", LA_Puzzle);
	lua_register(LS, "SetName", LA_Name);
	lua_register(LS, "_MainData", LA_MainData);
	lua_register(LS, "NonFatal", LA_NonFatal);
	lua_register(LS, "Object", LA_Object);
	lua_register(LS, "BreakBlock", LA_BreakBlock);
	lua_register(LS, "Laser", LA_Laser);
	lua_register(LS, "Death", LA_Death);
	luaL_loadstring(LS, "BPC_Test()"); lua_call(LS, 0, 0);
	if (!Script(ExtractDir(args[0]) + "/Import BallPlay Cupid.lua")) return 0;
	cout << "Creating: " << TarDir + "BallPlay Cupid" << endl;
	JT_Create Out{ TarDir + "BallPlay Cupid" };
	ScanTex(&Out);
	PuzzleDir = OriDir + "Stuff/Tricky Script/Script/Game.lll/Puzzles/";

	for (byte i = 1; i <= 50; i++) {
		MapMap[i] = CreateTeddy(25, 15, 32, 32, "PUZZLE", "WALL;BREAK;FLOOR;DIRECTIONS;BOMBS;GLASS");
		auto TR{ MapMap[i]->Rooms["PUZZLE"] };
		MapMap[i]->_MaxTiles = TeddyMaxTile::B16;
		TR->CreateZone("DEATH");
		TR->CreateZone("TRANS");
		
		char cmd[100]; sprintf_s(cmd, "Convert(%d)", i);
		luaL_loadstring(LS, cmd);
		lua_call(LS, 0, 0);
		char outf[500], outp[500];
		sprintf_s(outf, "Puzzles/Puz%02d", i);
		sprintf_s(outp, "Puz%02d", i);
		if (MapMap[i]->Data["Background"] != "*FUNCTION*") Fractal(&Out, i); else cout << "This map has a background function: " << MapMap[i]->Data["BackgroundFunction"] << "!\n";
		for (auto di : MapMap[i]->Data) { cout << di.first << " = " << di.second << endl; }
		//cout << "Adding to resource as: " << outf << endl;
		PID.Value("Puzzles", "Max", to_string(i));
		PID.Value("Puzzles", outp, MapMap[i]->Data["Title"]);
	}
	Tools(&Out);
	Objects(&Out);
	for (byte i = 1; i <= 50; i++) {
		char outf[500];
		sprintf_s(outf, "Puzzles/Puz%02d", i);
		TeddySave(MapMap[i], &Out, outf);
	}
	PID.Value("PUZCONFIG", "Glass", "YES");
	PID.Value("Meta", "Name", "BallPlay Cupid");
	PID.Value("Meta", "Author", "Jeroen P. Broks");
	PID.Value("Meta", "Created", "2016");
	PID.Value("Tech", "Original_Programming_Language", "Lua");
	PID.Value("Copyright", "License", "GPL3" );
	PID.Value("Copyright", "Copyright", "(c) Jeroen P. Broks 2016");
	Out.AddString("Meta.ini", PID.UnParse());
	Out.Close();
	lua_close(LS);
	Error("", false, true);
	return 0;
}