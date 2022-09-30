// Lic:
// BallPlay++
// Imports Puzzles from "BallPlay for Windows"
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
// Version: 22.09.23
// EndLic
#include <iostream>
#include <string>

#include <QuickTypes.hpp>
#include <QuickStream.hpp>
#include <GINIE.hpp>

#include <JCR6_core.hpp>

#include <SuperTed_Core.hpp>
#include <SuperTed_Save.hpp>

#include "../../Game/BallPlay++/headers/dir_obj.h"


using namespace std; // If you don't wanna get banned from my repositories, DON'T comment on me using that line!
using namespace TrickyUnits;
using namespace SuperTed;
using namespace jcr6;

//#define ww 16
//#define wh 11
#define ww 14
#define wh 12

namespace work {

	const string PackFile = "/Projects/Applications/Blitz Basic/BlitzBasic BallPlay/Assets/Ballplay.lvl";
	const string OutFile = "/Projects/Applications/VisualStudio/VC/BallPlay++/Game/JCR/Packages/BallPlay For Windows";
	const string Lic="Attribution - NonCommercial - NoDerivatives 4.0 International (https://creativecommons.org/licenses/by-nc-nd/4.0/)";
	const string PackHeader = "\rBallPlay For Windows 1.0 \n\r(C) Copyright JBC-Soft, Jeroen Broks, 2002, All rights reserved\x1a";
	const string TexDir = "/Projects/Applications/VisualStudio/VC/BallPlay++/ImportAssets/BlitzBasic BallPlay/";
	const uint32 UltimateMax = 50;

	const char* MissionName[5] = {"","Normal", "Break Away", "Break Free", "Color Splitting"};

	string		
		PackName,
		Author,
		LevelName[51];
	byte
		Plate1[51],
		Plate2[51],
		Wall[51],
		Bulldozer[51],
		Required[51],
		Veld[51][20][20];
	int
		MaxLevels,
		Mission[51],
		LvlBack[51],
		PwLen, Pw[501];


	bool LoadLevelPack() {
		// First the original Blitz Basic code for loading level packs.
		// This will help me to have what I need. All I need to do is to translate it to C++

		/* Function LoadLevelPack()
	Local BT  = ReadFile(LevelPack$)
	Local Lvl
	Local Ak,Al
	Local V1,V2
	Local H$
	Delete Each Objects
	SetFont Arial
	If Bt = 0 Then
		Cls
		Flip
		Print "Hmmm... I can't load: "+LevelPack$
		Print "You could have mispelled the filename, or there's a diskerror"
		Print "I'm very very sorry, pal!"
		Print
		Print "Jam any key to go back to the main menu!"
		FlushKeys
		WaitKey
		FlushKeys
		Return False
		EndIf
	V1 = ReadByte(BT)
	V2 = ReadByte(BT)
	If V1<>1 And V2<>0 Then
		Cls
		Flip
		Print "This level pack is created with a diffrent version"
		Print "It may contain codes, this version does not support!"
		Print
		Print
		Print "Jam any key to contine...."
		FlushKeys
		WaitKey
		FlushKeys
		Return False
		EndIf
	H$ = ReadString(BT)
	If H$<>PackHeader$ Then
		Cls
		Flip
		Print "This file is either corrupted, or somebody tried to be funny with it!"
		Print
		Print "Jam any key..."
		FlushKeys
		WaitKey
		FlushKeys
		Return False
		EndIf
	PackName$   = ReadString(BT)
	Author$	    = ReadString(BT)
	PwLen       = ReadInt(BT)
	For Ak = 1 To PwLen
		Pw(Ak)  = ReadInt(BT)
		Next
	MaxLevels   = ReadInt(BT)
	If MaxLevels > UltimateMax Then MaxLevels = 50
	For Lv = 1 To MaxLevels
		LevelName (Lv) = ReadString(BT)
		Mission   (Lv) = ReadInt   (BT)
		LvlBack   (Lv) = ReadInt   (BT)
		Plate1    (Lv) = ReadByte  (BT)
		Plate2    (Lv) = ReadByte  (BT)
		Wall      (Lv) = ReadByte  (BT)
		Bulldozer (Lv) = ReadByte  (BT)
		Required  (Lv) = ReadByte  (BT)
		For Ak = 0 To 16
			For Al = 0 To 11
				Veld(Lv,Ak,Al) = ReadByte(BT)
				Next
			Next
		Next
	CloseFile BT
	Return True
	End Function
		*/

		cout << "Reading: " << PackFile << endl;
		auto BT{ ReadFile(PackFile) }; 

		// Header
		auto
			V1{ BT->ReadByte() },
			V2{ BT->ReadByte() };
		printf("=> Version check: %d.%02d\n", V1, V2);
		if (V1 != 1 || V2 != 0) {
			//Cls
			//Flip
			cout << "This level pack is created with a diffrent version (" << V1 << "." << V2 << ")\n";
			cout << "It may contain codes, this version does not support!" << endl;
			return false;
		}
		cout << "=> Check header\n";
		auto H{ BT->ReadString() };
		if (H != PackHeader) {
			cout << "This file is either corrupted, or somebody tried to be funny with it!" << endl;
			return false;
		}

		// Meta data
		PackName = BT->ReadString(); cout << "=> Pack name: " << PackName << endl;
		Author = BT->ReadString(); cout << "=> Author: " << Author << endl;
		
		// Password protection (Against unauthorized modifications. Not important anymore, but must be read to prevent trouble).
		PwLen = BT->ReadInt();
		cout << "=> Pw(" << PwLen << ")\t";
		for (uint32 Ak = 1; Ak <= PwLen; Ak++) {
			Pw[Ak] = BT->ReadInt();
			if (Ak > 1) cout << ", ";
			cout << Pw[Ak];
		}
		cout << endl;

		// Levels themselves
		MaxLevels = BT->ReadInt();
		if (MaxLevels > UltimateMax) MaxLevels = 50;
		for (uint32 Lv = 1; Lv <= MaxLevels; Lv++) {
			printf("=> Puzzle %02d/%02d: ", Lv, MaxLevels);
			LevelName[Lv] = BT->ReadString();
			Mission[Lv] = BT->ReadInt();
			printf(" \"%s\" (%s)\n", LevelName[Lv].c_str(), MissionName[Mission[Lv]]);
			LvlBack[Lv] = BT->ReadInt();
			Plate1[Lv] = BT->ReadByte();
			Plate2[Lv] = BT->ReadByte();
			Wall[Lv] = BT->ReadByte();
			Bulldozer[Lv] = BT->ReadByte();
			Required[Lv] = BT->ReadByte();
			// Please note. 16 must be for the width here or loading will spook up
			for (byte Ak = 0; Ak <= 16; ++Ak) { //For Ak = 0 To 16
				if (Ak && Ak % 4 == 0) cout << endl;
				for (byte Al = 0; Al <= 11; ++Al) {
					if (Al && Al % 4 == 0) cout << " ";
					Veld[Lv][Ak][Al] = BT->ReadByte();
					printf("%02x ",Veld[Lv][Ak][Al]);
				}
				cout << "\n";
			}
		}

		cout << "=> All Done\n\n";
		BT->Close();
		return true;
	}

	map<uint32, string> Imported;
	map<uint32, string> Tex;
	void Import(JT_Create*Out,uint32 idx,string As="",uint32 TexIndex=0, bool NoTex=false){
		if (!Imported.count(idx)) {
			char TexName[200]; sprintf_s(TexName, "BlitzTex_%02x.bmp",idx);
			if (As == "") As = TexName;
			cout << "=> Importing tex  \"" << TexName << "\" as \"" << As << "\"\n";
			if (TexIndex == 0) TexIndex = idx;
			Imported[idx] = As;
			if (!NoTex) Tex[TexIndex] = "Packages/BallPlay For Windows/Textures/"+As+".bmp";
			auto TexEntry{ Out->AddFile(TexDir + TexName, "Textures/" + As + ".bmp") };
			TexEntry.dataString["__Author"] = "Jeroen P. Broks";
			TexEntry.dataString["__Notes"] = Lic;
		}
	}

	int
		Failed{ 0 },
		Succeeded{ 0 };
	void Convert(JT_Create* Out, uint32 puz) {
		printf("=> Puzzle %02d/%02d - %s (%s)\n", puz, 50, LevelName[puz].c_str(),MissionName[Mission[puz]]);
		auto TM{ CreateTeddy(ww,wh,40,40,"PUZZLE","WALL;BREAK;FLOOR;DIRECTIONS;BOMBS") };
		auto TR{ TM->Rooms["PUZZLE"] };
		TR->CreateZone("DEATH");
		TR->CreateZone("TRANS");

		// Clean Up
		cout << "=> Cleaning up: ";
		for (auto lay : TR->Layers) {
			cout << lay.first << "; ";
			for (byte x = 0; x < ww; x++) for (byte y = 0; y < wh; y++) lay.second->Field->Value(x, y, 0);
		}
		cout << endl;

		TM->Data["Title"] = LevelName[puz];
		TM->Data["Mission"] = MissionName[Mission[puz]];
		TM->Data["BackGround"] = LvlBack[puz];
		TM->Data["Plate/"] = to_string(Plate1[puz]);
		TM->Data["Plate\\"] = to_string(Plate2[puz]);
		TM->Data["Plate1"] = TM->Data["Plate/"];
		TM->Data["Plate2"] = TM->Data["Plate\\"];
		TM->Data["Required"] = to_string(Required[puz]);
		TM->Data["Bulldozer"] = to_string(Bulldozer[puz]);
		TM->Data["Remove"] = to_string(Bulldozer[puz]);
		TM->Data["Barrier"] = to_string(Wall[puz]);
		//*
		if ((!TM->Textures.count(1)) || (!TM->Textures[1])) {
			TM->Textures[1]=std::make_shared<_TeddyTex>();
			TM->Textures[1]->TexFile = "Packages/BallPlay For Windows/Tools/Barrier.png";
			TM->Textures[1]->r = 255;
			TM->Textures[1]->g = 255;
			TM->Textures[1]->b = 255;
			TM->Textures[1]->alpha = 255;
			Tex[1] = TM->Textures[1]->TexFile;
		}
		//*/
		char back[500]; sprintf_s(back, "Packages/BallPlay For Windows/Backgrounds/Back%04d.bmp", (puz % 20) + 1);
		TM->Data["Background"] = back;

		for (byte x = 0; x < ww; x++) for (byte y = 0; y < wh; y++) {
			auto VV{ Veld[puz][x][y] };
			switch (VV) {
				// Nothing
			case 0:
				break;
				// Ball
			case 0x10: {
				Import(Out, 0x10, "Ball", 0, true);
				auto ball = TR->AddObject(x, y, 1);
				ball->Data["Direction"] = "South";
			} break;
			case 0x13: {
				Import(Out, 0x13, "Ball Green", 0, true);
				auto ball = TR->AddObject(x, y, 2);
				ball->Data["Direction"] = "South";
			
			} break;
			case 0x14: {
				Import(Out, 0x14, "Ball Red", 0, true);
				auto ball = TR->AddObject(x, y, 3);
				ball->Data["Direction"] = "South";			
			} break;
				// Ghost
			case 0x12: {
				Import(Out, 0x12, "Ghost_Grey", 0, true);				
				auto ghost = TR->AddObject(x, y, 4);
				ghost->Data["Direction"] = "South";
				ghost->Data["Color"] = "Grey";
				ghost->Data["Red"] = "255";
				ghost->Data["Green"] = "255";
				ghost->Data["Blue"] = "255";
				ghost->Data["Alpha"] = "255";
			} break;
				// Droid
			case 0x11: {
				Import(Out, 0x11, "Droid", 0, true);
				auto droid = TR->AddObject(x, y, BallPlay::Droid);
				droid->Data["Direction"] = "South";
				
			} break;


				// Directional
			case 0x20:
			case 0x21:
			case 0x22:
			case 0x23:
			case 0x24: // nobuild
			case 0x25:
			case 0x26:
			case 0x27:
			case 0x28:
			case 0x29:
			case 0x2a:
			case 0x2b:
			case 0x2c:
			case 0x2d:
				Import(Out, 0x20, "Plate_1");
				Import(Out, 0x21, "Plate_2");
				//Import(Out, VV + 2, "Direction_" + to_string((VV + 2) % 16));
				//TR->Layers["DIRECTIONS"]->Field->Value(x, y, VV+2);
				Import(Out, VV, "Direction_" + to_string((VV) % 16));
				TR->Layers["DIRECTIONS"]->Field->Value(x, y, VV);
				break;
				// Transporters
			case 0x30:
			case 0x31:
			case 0x32:
			case 0x33:
			case 0x34:
			case 0x35:
			case 0x36:
			case 0x37:
			case 0x38:
			case 0x39:
			case 0x3a:
			case 0x3b:
			case 0x3c:
			case 0x3d:
			case 0x3e:
			case 0x3f:
				Import(Out, VV, "Trans_" + right("0" + to_string((VV % 16) + 1), 2));
				TR->Layers["FLOOR"]->Field->Value(x, y, VV);
				TR->Layers["TRANS"]->Field->Value(x, y, VV);
				break;
				// Wall
			case 0x40:
			case 0x41:
				Import(Out, VV,"Wall_"+to_string((VV%16)+1));
				TR->Layers["WALL"]->Field->Value(x, y, VV);
				break;
				// Break-Away
			case 0x50:
			case 0x51:
			case 0x52:
			case 0x53:
			case 0x54:
			case 0x55:
			case 0x56:
			case 0x57:
			case 0x58:
			case 0x59:
			case 0x5a:
			case 0x5b:
			case 0x5c:
			case 0x5d:
			case 0x5e:
			case 0x5f:
				Import(Out, VV, "BreakAway_" + right("0"+to_string((VV % 16) + 1),2));
				TR->Layers["BREAK"]->Field->Value(x, y, VV);
				break;
				// Water
			case 0x60:
				Import(Out, VV, "Water");
				TR->Layers["DIRECTIONS"]->Field->Value(x, y, VV);
				TR->Layers["DEATH"]->Field->Value(x, y, 1);
				break;
				// Mine
			case 0x61:
				Import(Out, VV, "Mine");
				TR->Layers["BOMBS"]->Field->Value(x, y, VV);
				break;
				// Exit
			case 0x70:
			case 0x71:
			case 0x72:
				Import(Out, VV, "Exit_" + to_string((VV % 16) + 1));
				TR->Layers["DIRECTIONS"]->Field->Value(x, y, VV);
				break;
			default:
				printf("FAILED! Unknown Veld Tag %03d/%02x at (%02d,%02d)\n", VV,VV, x, y);
				Failed++;
				return;
			}
		}
		for (uint32 i = 1; i < 256; i++) { TM->Textures[i] = make_shared<_TeddyTex>(); TM->Textures[i]->TexFile = Tex[i]; }
		char PName[300];
		sprintf_s(PName, "Puzzles/Puz%02d", puz);
		TeddySave(TM, Out, PName);
		Succeeded++;
	}

	void ConvertPack() {
		GINIE Meta{};
		Meta.Value("Meta", "Name", "BallPlay For Windows");
		Meta.Value("Meta", "Author", "Jeroen P. Broks");
		Meta.Value("Meta", "Created", "2001");
		Meta.Value("Tech", "Original_Programming_Language", "BlitzBasic");
		Meta.Value("Copyright", "License", Lic);
		Meta.Value("Copyright", "Copyright", "(c) Jeroen P. Broks 2001");
		Meta.Value("Game", "Puzzles", "50");
		Meta.Value("Death", "MAX", 1);
		Meta.Value("Death", "001", "Splash");
		cout << "Creating output package: " << OutFile << endl;
		JT_Create Out{ OutFile };
		cout << "=> Meta\n";
		Meta.Value("Puzzles", "Max", "50");
		for (byte i = 1; i <= 50; i++) {
			char pz[10]; sprintf_s(pz, "PUZ%02d", i);			
			Meta.Value("Puzzles", pz, LevelName[i]);
			Convert(&Out, i);
		}
		Out.AddString("Meta.ini", Meta.UnParse());
		for (byte i = 1; i <= 20; i++) {
			char addFile[300], asFile[300];
			sprintf_s(addFile, "/Projects/Applications/VisualStudio/VC/BallPlay++/ImportLevels/Project1Import BlitzBasic BallPlay/back/Back%04d.bmp", i);
			sprintf_s(asFile, "Backgrounds/Back%04d.bmp", i);
			Out.AddFile(addFile, asFile);
			cout << "Adding '" << addFile << "' as '" << asFile << "'\n";
		}
		Out.Alias("Textures/Ball.bmp", "Objects/Ball.png"); // the extensions have no value for the system.
		Out.Alias("Textures/Ghost_Grey.bmp", "Objects/Ghost.png");
		Out.Alias("Textures/Droid.bmp", "Objects/Droid.png");
		Out.Alias("Textures/Plate_1.bmp", "Tools/Plate1.png");
		Out.Alias("Textures/Plate_2.bmp", "Tools/Plate2.png");
		Out.Alias("Textures/Wall_1.bmp", "Tools/Barrier.png");
		Out.AddFile("E:/Projects/Applications/VisualStudio/VC/BallPlay++/ImportAssets/BlitzBasic BallPlay/BlitzTex_04.bmp", "Tools/Remove.png");
		Out.Close();
		printf("Done - Success: %02d; Failures: %02d\n", Succeeded, Failed);
	}
}

int main(int n, char** arg) {
	init_JCR6();
	if (!work::LoadLevelPack()) {
		cout << "Loading failed\n";  return 1;
	}
	work::ConvertPack();
	return 0;
}