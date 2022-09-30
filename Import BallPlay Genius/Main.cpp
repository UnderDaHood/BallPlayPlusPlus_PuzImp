// Lic:
// BallPlay++
// Ballplay Genius Puzzle Import
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
// Version: 22.09.24
// EndLic
#include <memory>
#include <string>
#include <map>

#include <QuickStream.hpp>
#include <jcr6_jxsda.hpp>
#include <jcr6_core.hpp>
#include <QuickTypes.hpp>
#include <GINIE.hpp>

#include <SuperTed_Save.hpp>

#include <iostream>

#include "../../Game/BallPlay++/headers/dir_obj.h"

using namespace std;
using namespace TrickyUnits;
using namespace jcr6;

namespace BPG_Import {


	const int
		PE_MaxX{ 100 },
		PE_MaxY{ 100 },
		PE_MaxLevels{ 500 },
		PE_MaxPuzzles{ PE_MaxLevels };

	string 
		PE_Game{ "Unknown Game" };
	int
		PE_DefWidth{ 10 }, //		' Default number of tiles in width
		PE_DefHeight{ 10 },//		' Default number of tiles in height
		PE_TileWidth{ 40 },//		' Default tilewidth in pixels
		PE_TileHeight{ 40 };//		' Default tileheight in pixels

	const char LayNames[3][200]{ "Walls + Breakaway","Special objects (like plates and stuff)","Floor" };
	const string OutFile = "/Projects/Applications/VisualStudio/VC/BallPlay++/Game/JCR/Packages/BallPlay Genius";
	const string IncBin = "/Projects/Applications/VisualStudio/VC/BallPlay++/ImportLevels/Import BallPlay Genius/Original/IncBin/Gfx/";

	// Shared pointers. I am not sure if they are really needed, but this way I can make sure that a> all data always points correctly and b> I can easier cling on the original BlitzMax standard with this!
	class _PackRec; typedef shared_ptr<_PackRec> PackRec;
	class _PuzzleRec; typedef shared_ptr<_PuzzleRec> PuzzleRec;

	GINIE PID;

	class _PuzzleRec {
	public:
		int
			F[3][PE_MaxX][PE_MaxY],		// Field Record 100x100 is maximum supported in this version
			Par{ 0 },			  		// Par time in seconds
			DefaultFormat{ 1 },		// Default Format. when set to 1 it uses the DefWidth and DefHeight settings automatically in Height and Width
			Height{ 0 }, Width{ 0 };			// Height and width. Automatically set when Defeault format is 1
		string 
			Name;					// Puzzle Name	
		map<string, string> 
			ExtraData; //: TMap = New TMap	' Used to store extra data that is not standard for PuzzleEditor
	};

	class _PackRec {
	public:
		static PackRec NEW() { return make_shared<_PackRec>(); }
		int
			Owner_ID[10]{ 0,0,0,0,0,0,0,0,0,0 },
			Game_ID[10]{ 0,0,0,0,0,0,0,0,0,0 },
			Pack_ID[10]{ 0,0,0,0,0,0,0,0,0,0 },
			Puzzles; //' Number of puzzle
		PuzzleRec Puzzle[PE_MaxLevels];
		string Author{ "" };
		bool
			Open{ true },//Field Open = 1 ' if set to 1 then the author allows editting by others
			Yours{ false }; // May or may not be needed.
	
		_PackRec() {
			for (uint32 ak = 0; ak < PE_MaxLevels; ak++)
				Puzzle[ak] = make_shared<_PuzzleRec>();			
		}
	};

	PackRec PE_Load(string file, bool MustExist = false) {

		/* First the original source code in BlitzMax unaltered
		   I need that as referrence stuff, you know!

	Function PE_Load :PackRec(File$, MustExist = 0)
		Local BT : TStream = ReadFile(File)
		Local Ret : PackRec = New PackRec
		Local AK, X, Y, Ok, Lengte, Key$, Value$
		Local C
		Local CL = 0
		For ak = 0 To PE_MaxLevels - 1
		For Local Lay = 0 To 2
		For x = 0 To PE_maxX - 1
		For y = 0 To PE_MaxY - 1
		'Print "Clean record: "+Ak
		Ret.Puzzle[Ak].F[Lay, X, Y] = 0
		Next
		Next
		Next
		Next
		If BT
		BT = LittleEndianStream(BT)
		If ReadString(BT, Len("PUZZLEPACK")) < > "PUZZLEPACK"
		Notify "The requested file does not appear to be a puzzle pack"
		CloseFile BT
		Return Null
		EndIf
		Print "Loading Puzzle Pack: " + File
		While Not Eof(BT)
		C = ReadByte(BT)
		'Print "Read Command Tag: "+C
		Select C
		Case 0
		Lengte = ReadInt(BT)
		Ret.Author = ReadString(BT, Lengte)
		Print "Pack.Author = ~q" + Ret.Author + "~q"
		Case 1
		For ak = 0 To 9
		If PE_Current_Game[Ak]<>ReadInt(BT)
		Notify "The requested file appears to have been created for a different game"
		CloseFile BT
		Return Null
		EndIf
		Next
		Case 2
		For Ak = 0 To 9
		Ret.Owner_ID[Ak] = ReadInt(BT)
		Next
		Case 3
		Ret.Open = ReadByte(BT)
		Case 4
		Ret.Puzzles = ReadInt(BT)
		Case 5
		CL = ReadInt(BT)
		Case 6
		Lengte = ReadInt(BT)
		Ret.Puzzle[CL].Name = ReadString(BT, Lengte)
		Case 7
		Ret.Puzzle[CL].Par = ReadInt(BT)
		Case 8
		Ret.Puzzle[CL].DefaultFormat = ReadByte(BT)
		Case 9
		Ret.Puzzle[CL].Width = ReadInt(BT)
		Ret.Puzzle[CL].Height = ReadInt(BT)
		If Ret.Puzzle[CL].DefaultFormat And Ret.Puzzle[CL].Width<>PE_DefWidth And Ret.Puzzle[CL].Height<>PE_DefHeight
		Notify "Warning!~nLevel " + CL + " is said to be written in standard format~n~nHowever the size does not match the default settings of this game.~nPuzzle:(" + Ret.Puzzle[CL].Width + "x" + Ret.Puzzle[CL].Width + "), Default:(" + PE_DefWidth + "," + PE_DefHeight + ")~nThis could indicate that either this file is corrupted or that it's written with a higher version of this game.~nI'll try to continue the loading, but but be prepared for some errors!"
		EndIf
		If Ret.Puzzle[Cl].Width > PE_MaxX Or Ret.Puzzle[CL].Height > PE_MaxY
		Notify "Error!~nLevel " + CL + " exceeds the maximum format this version allows.~nPuzzle:(" + Ret.Puzzle[CL].Width + "x" + Ret.Puzzle[CL].Width + "), Max:(" + PE_MaxX + "," + PE_MaxY + ")"
		CloseFile BT
		Return Null
		EndIf
		Rem
		Case 10 ' Foutief systeem. Vervallen, goede systeem is nu 11
		For Y = 0 To Ret.Puzzle[Cl].Height - 1
		For X = 0 To Ret.Puzzle[Cl].Width - 1
		Ret.Puzzle[Cl].F[0, X, Y] = ReadInt(BT)
		Next
		Next
		End Rem
		Case 11
		For Local Lay = 0 To 2
		For Y = 0 To Ret.Puzzle[Cl].Height - 1
		For X = 0 To Ret.Puzzle[Cl].Width - 1
		Ret.Puzzle[Cl].F[Lay, X, Y] = ReadInt(BT)
		Next
		Next
		Next
		Case 12
		lengte = ReadInt(BT)
		Key = ReadString(BT, Lengte)
		Lengte = ReadInt(BT)
		Value = ReadString(BT, Lengte)
		MapInsert Ret.Puzzle[Cl].ExtraData, Key, Value
		Case 13
		For Ak = 0 To 9
		Ret.Pack_ID[Ak] = ReadInt(BT)
		Next
		Default
		Notify "The requested file is either corrupted or written with a newer version (" + C + ")"
		CloseFile BT
		Return Null
		End Select
		Wend
		CloseStream BT
		Else
		If MustExist
		Notify "File not found!"
		Return Null
		EndIf
		Ret.Puzzles = 1
		Ret.Author = PE_Current_Author
		For ak = 0 To 9
		Ret.Owner_ID[Ak] = PE_Current_Owner[Ak]
		Ret.Game_ID[Ak] = PE_Current_Game[Ak]
		Ret.Pack_ID[Ak] = Rand(1, MilliSecs())
		Next
		For ak = 0 To PE_MaxLevels - 1
		For Local Lay = 0 To 2
		For x = 0 To PE_maxX - 1
		For y = 0 To PE_MaxY - 1
		Ret.Puzzle[Ak].F[Lay, X, Y] = 0
		Next
		Next
		Next
		Next
		Ret.Open = 1
		EndIf
		Ok = True
		For ak = 0 To 9
		If Ret.Owner_ID[Ak]<>PE_Current_Owner[Ak] Ok = False
		Next
		Ret.Yours = Ok
		Return ret
		End Function
		*/

		// And now to translate all this crap to C++
cout << "Loading: " << file << "\n";
		auto BT{ ReadFile(file) };
		auto Ret{ _PackRec::NEW() };
		//Local AK, X, Y, Ok, Lengte, Key$, Value$ // I won't do them all, as some will remain local in for-loops. Something ? was pretty inconsequent in back then, as the languages I used so far didn't support it
		bool Ok{ false };
		int Lengte{ 0 };
		string Key{ "" }, Value{ "" };
		byte C{ 0 };
		int CL{ 0 };


		// Cleaning all field variables. BlitzMax should have done that automatically (contrary to C++)
		// So I wonder why I put this in back then. No matter, it makes this convertation only easier on me, so why complain?
		for (uint32 ak = 0; ak < PE_MaxLevels; ++ak) {
			for (byte Lay = 0; Lay <= 2; ++Lay) {
				for (byte x = 0; x < PE_MaxX; ++x) {
					for (byte y = 0; y < PE_MaxY; ++y) {
						//cout << "Clean record: " << ak << endl;
						Ret->Puzzle[ak]->F[Lay][x][y] = 0;
					}
				}
			}
		}

		//If BT
			//BT = LittleEndianStream(BT) // I only used this on little endian anyway
		if (BT->ReadString(strlen("PUZZLEPACK")) != "PUZZLEPACK") {
			cout << "The requested file does not appear to be a puzzle pack\n";
			BT->Close();
			return nullptr;
		}


		cout << "Loading Puzzle Pack: " << file << endl;
		while (!BT->EndOfFile()) { // While Not Eof(BT)
			C = BT->ReadByte();
			//'Print "Read Command Tag: "+C
			switch (C) {
			case 0:
				Lengte = BT->ReadInt();
				Ret->Author = BT->ReadString(Lengte);
				cout << "Pack.Author = \"" << Ret->Author << "\"" << endl;
				break;
			case 1:
				for (byte ak = 0; ak <= 9; ++ak) {
					/* Not gonna do this checkup anymore, thank you!
						If PE_Current_Game[Ak]<>BT->ReadInt();
						Notify "The requested file appears to have been created for a different game"
						CloseFile BT
						Return Null
						EndIf
					Next
					*/
					BT->ReadInt(); // I had to read the values, but I ain't gonna check 'm anymore!
				}
				break;
			case 2:
				// Only gonna read this to make sure the rest of the loading process won't spook up.
				for (byte Ak = 0; Ak <= 9; Ak++) {
					Ret->Owner_ID[Ak] = BT->ReadInt();
				}
				break;
			case 3:
				Ret->Open = BT->ReadByte();
				break;
			case 4:
				Ret->Puzzles = BT->ReadInt();
				cout << "This pack contains " << Ret->Puzzles << " puzzles" << endl;
				break;
			case 5:
				CL = BT->ReadInt();
				break;
			case 6:
				Lengte = BT->ReadInt();
				Ret->Puzzle[CL]->Name = BT->ReadString(Lengte);
				cout << "Puzzle #" << CL << ": " << Ret->Puzzle[CL]->Name << endl;
				break;
			case 7:
				Ret->Puzzle[CL]->Par = BT->ReadInt();
				break;
			case 8:
				Ret->Puzzle[CL]->DefaultFormat = BT->ReadByte();
				break;
			case 9:
				Ret->Puzzle[CL]->Width = BT->ReadInt();
				Ret->Puzzle[CL]->Height = BT->ReadInt();
				if (Ret->Puzzle[CL]->DefaultFormat && Ret->Puzzle[CL]->Width != PE_DefWidth && Ret->Puzzle[CL]->Height != PE_DefHeight) {
					cout << "Warning!\nLevel " << CL << " is said to be written in standard format\n\nHowever the size does not match the default settings of this game.\nPuzzle:(" << Ret->Puzzle[CL]->Width << "x" << Ret->Puzzle[CL]->Width << "), Default:(" << PE_DefWidth << "," << PE_DefHeight << ")\nThis could indicate that either this file is corrupted or that it's written with a higher version of this game.\nI'll try to continue the loading, but but be prepared for some errors!\n";
				}
				if (Ret->Puzzle[CL]->Width > PE_MaxX || Ret->Puzzle[CL]->Height > PE_MaxY) {
					cout << "Error!\nLevel " << CL << " exceeds the maximum format this version allows.\nPuzzle:(" << Ret->Puzzle[CL]->Width << "x" << Ret->Puzzle[CL]->Width << "), Max:(" << PE_MaxX << "," << PE_MaxY << ")\n";
					BT->Close();
					return nullptr;
				}
				break;
				/*
				Rem
				Case 10 ' Foutief systeem. Vervallen, goede systeem is nu 11
				For Y = 0 To Ret->Puzzle[Cl].Height - 1
				For X = 0 To Ret->Puzzle[Cl].Width - 1
				Ret->Puzzle[Cl].F[0, X, Y] = BT->ReadInt();
				Next
				Next
				End Rem
				*/
			case 11:
				for (byte Lay = 0; Lay <= 2; ++Lay) {
					printf("Layer #%d - %s\n", Lay,LayNames[Lay]);
					for (byte Y = 0; Y < Ret->Puzzle[CL]->Height; ++Y) {
						for (byte X = 0; X < Ret->Puzzle[CL]->Width; ++X) {
							Ret->Puzzle[CL]->F[Lay][X][Y] = BT->ReadInt();
							if (Lay == 0 && Ret->Puzzle[CL]->F[Lay][X][Y] >= 9000)
								printf("\x1b[41m%04x\x1b[0m ", Ret->Puzzle[CL]->F[Lay][X][Y]);
							else
								printf("%04x ", Ret->Puzzle[CL]->F[Lay][X][Y]);
						}
						printf("\n");
					}
				}
				break;
			case 12:
				Lengte = BT->ReadInt();
				Key = BT->ReadString(Lengte);
				Lengte = BT->ReadInt();
				Value = BT->ReadString(Lengte);
				Ret->Puzzle[CL]->ExtraData[Key] = Value; //MapInsert Ret->Puzzle[Cl].ExtraData, Key, Value
				break;
			case 13:
				for (byte Ak = 0; Ak <= 9; ++Ak) {
					Ret->Pack_ID[Ak] = BT->ReadInt();
				}
				break;
			default:
				cout << "The requested file is either corrupted or written with a newer version (" << C << ")\n";
				BT->Close();//CloseFile BT
				return nullptr;
			}
		}

		// Last part
		BT->Close();
		/*
			Else
			If MustExist
			Notify "File not found!"
			Return Null
			EndIf
			Ret.Puzzles = 1
			Ret.Author = PE_Current_Author
			For ak = 0 To 9
			Ret.Owner_ID[Ak] = PE_Current_Owner[Ak]
			Ret.Game_ID[Ak] = PE_Current_Game[Ak]
			Ret.Pack_ID[Ak] = Rand(1, MilliSecs())
			Next
			For ak = 0 To PE_MaxLevels - 1
			For Local Lay = 0 To 2
			For x = 0 To PE_maxX - 1
			For y = 0 To PE_MaxY - 1
			Ret.Puzzle[Ak].F[Lay, X, Y] = 0
			Next
			Next
			Next
			Next
			Ret.Open = 1
			EndIf
			*/

		Ok = true;
		//	For ak = 0 To 9
			//If Ret.Owner_ID[Ak]<>PE_Current_Owner[Ak] Ok = False
		//Next
		Ret->Yours = Ok; // Check not important here.


		return Ret;
	}

	std::string Import(JT_Create* Out,std::string tex) {
		static map<string, string> _tex;
		auto utex = Upper(tex);
		if (!_tex.count(utex)) {
			cout << "Importing texture: " << tex<<endl;
			char ff[300];
			char tf[300];
			sprintf_s(ff, "%s%s", IncBin.c_str(), tex.c_str());
			sprintf_s(tf, "Textures/%s", tex.c_str());
			_tex[utex] = string("Packages/BallPlay Genius/") + tf;
			if (!FileExists(ff)) {
				printf("\x1b[31mError!\x1b[0m\t File not found: %s\n", ff);
			}
			if (ExtractExt(Upper(ff)) == "JPBF") {
				auto In{ Dir(ff) };
				for (auto a : In.Entries()) {
					auto b{ In.B(a.first) };
					auto c{ Out->StartEntry(string(tf) + "/" + a.first) };
					while (!b->eof()) c->Write(b->ReadByte());
					c->Close();
				}
			
			} else
				Out->AddFile(ff, tf);
		}
		return _tex[utex];
	}
#define Laser(ncase,wind,dir,cr,cg,cb)\
	case ncase: {\
		TM->Tex(ncase)->TexFile = Import(Out,string("Laser")+string(dir)+".png");\
		TM->Tex(ncase)->r=cr;\
		TM->Tex(ncase)->g=cg;\
		TM->Tex(ncase)->b=cb;\
		TR->Layers["WALL"]->Field->Value(x,y,ncase);\
		auto olaser = TR->AddObject(x, y, laser);\
		char coltag[10]; sprintf_s(coltag,"%02x%02x%02x",cr,cg,cb);\
		olaser->Data["Direction"] = wind;\
		olaser->Data["Tag"] = coltag;\
		olaser->Data["R"] = std::to_string(cr);\
		olaser->Data["G"] = std::to_string(cg);\
		olaser->Data["B"] = std::to_string(cr);\
		}\
		break
#define LaserPlate(ncase,cr,cg,cb)\
	case ncase:{\
		Import(Out, "LaserPlate.png");\
		auto pl{ TR->AddObject(x,y,lasertrigger)};\
		char coltag[10]; sprintf_s(coltag, "%02x%02x%02x", cr, cg, cb); \
		pl->Data["Tag"] = coltag; \
		pl->Data["R"] = std::to_string(cr); \
		pl->Data["G"] = std::to_string(cg); \
		pl->Data["B"] = std::to_string(cr); \
		}\
		break

#define MGhost(ncase,Wind,cr,cg,cb)\
	case ncase: {\
		Import(Out, "Ghost.png");\
		auto g{TR->AddObject(x,y,Ghost)};\
		g->Data["R"] = std::to_string(cr); \
		g->Data["G"] = std::to_string(cg); \
		g->Data["B"] = std::to_string(cr); \
		g->Data["Wind"] = Wind;\
		}\
		break
		


	void Convert(JT_Create* Out, PackRec Pck, byte puz) {
		using namespace SuperTed;
		using namespace BallPlay;
		auto cpz{ Pck->Puzzle[puz] };
		if (cpz->ExtraData["!Task"] == "Wall Breaker") cpz->ExtraData["!Task"] = "Break Away";
		printf("=> Puzzle: \x1b[32m%02d/%02d\x1b[0m -\x1b[33m %s\x1b[0m (%s)\n", puz+1, 50, cpz->Name.c_str(), cpz->ExtraData["!Task"].c_str());
		auto TM{ CreateTeddy(20,20,40,40,"PUZZLE","WALL;BREAK;FLOOR;DIRECTIONS;BOMBS") };
		auto TR{ TM->Rooms["PUZZLE"] };
		TM->_MaxTiles = TeddyMaxTile::B16;
		TR->CreateZone("DEATH");
		TR->CreateZone("TRANS");

		// Clean Up
		cout << "=> Cleaning up: ";
		for (auto lay : TR->Layers) {
			cout << lay.first << "; ";
			for (byte x = 0; x < 20; x++) for (byte y = 0; y < 20; y++) 
				lay.second->Field->Value(x, y, 0);
		}
		cout << endl;
		//for (auto chk : TM->Data) cout << "Data[\"" << chk.first << "\"] = " << chk.second << endl;
		for (auto chk : cpz->ExtraData) cout << "cpz->ExtraData[\"" << chk.first << "\"]=\"" << chk.second << "\";\n";
		TM->Data["Title"] = cpz->Name;
		TM->Data["Mission"] = cpz->ExtraData["!Task"];
		//TM->Data["BackGround"] = LvlBack[puz];
		TM->Data["Plate/"] = cpz->ExtraData["%Plate_/"];
		TM->Data["Plate\\"] = cpz->ExtraData["%Plate_\\"];
		TM->Data["Plate1"] = TM->Data["%Plate/"];
		TM->Data["Plate2"] = TM->Data["%Plate\\"];
		TM->Data["Barrier"] = cpz->ExtraData["%Wall"];  TM->Tex(1)->TexFile = "Packages/BallPlay Genius/Tools/Barrier.png";
		TM->Data["Required"] = cpz->ExtraData["%Required_balls"];
		TM->Data["Bulldozer"] = cpz->ExtraData["%Remove"];
		TM->Data["Remove"] = cpz->ExtraData["%Remove"];
		char back[500]; sprintf_s(back, "Packages/BallPlay Genius/Backgrounds/Fractal%d.png", (puz % 10) );
		TM->Data["Background"] = back;
		TM->Tex(userplate1)->TexFile = Import(Out, "Plate1.png");
		TM->Tex(userplate1)->g = 0;
		TM->Tex(userplate1)->b = 0;
		TM->Tex(userplate2)->TexFile = Import(Out, "Plate2.png");
		TM->Tex(userplate2)->g = 0;
		TM->Tex(userplate2)->b = 0;

		static string w[4] = { "North","South","West","East" };
		for (byte Lay = 0; Lay < 3; Lay++) {
			for (byte x = 0; x < cpz->Width; x++) {
				for (byte y = 0; y < cpz->Height; y++) {
					switch (Lay) {
						// Wall
					case 0:
						switch (cpz->F[Lay][x][y]) {
						case 0: break;
						case 1:
							TR->Layers["WALL"]->Field->Value(x, y, 1001);
							TM->Tex(1001)->TexFile = Import(Out, "GeneralWall.png");
							break;
						case 1000:
						case 1001:
						case 1002:
						case 1003: {
							int id{ cpz->F[Lay][x][y] + 2 };
							char muur[200];
							sprintf_s(muur, "Muur%d.png", cpz->F[Lay][x][y] - 999);
							TR->Layers["WALL"]->Field->Value(x, y, id);
							TM->Tex(id)->TexFile = Import(Out, muur);
							break;
						}
						case 5000:
							TM->Tex(1006)->TexFile = Import(Out, "PlantMuur.png");
							TR->Layers["WALL"]->Field->Value(x, y, 1006);
							break;
						case 5001:
							TM->Tex(1007)->TexFile = Import(Out, "PMW.png");
							TM->Tex(1007)->r = 0;
							TM->Tex(1007)->g = 0;
							TR->Layers["WALL"]->Field->Value(x, y, 1007);
							break;
							// Laser
							Laser(7000, "North", "Up", 0, 0, 255);
							Laser(7001, "North", "Up", 255, 0, 0);
							Laser(7002, "North", "Up", 0, 255, 0);
							Laser(7003, "North", "Up", 255, 0, 255);
							Laser(7004, "North", "Up", 0, 255, 255);
							Laser(7005, "North", "Up", 255, 255, 0);
							Laser(7006, "North", "Up", 255, 255, 255);

							Laser(7010, "East", "Right", 0, 0, 255);
							Laser(7011, "East", "Right", 255, 0, 0);
							Laser(7012, "East", "Right", 0, 255, 0);
							Laser(7013, "East", "Right", 255, 0, 255);
							Laser(7014, "East", "Right", 0, 255, 255);
							Laser(7015, "East", "Right", 255, 255, 0);
							Laser(7016, "East", "Right", 255, 255, 255);

							Laser(7020, "South", "Down", 0, 0, 255);
							Laser(7021, "South", "Down", 255, 0, 0);
							Laser(7022, "South", "Down", 0, 255, 0);
							Laser(7023, "South", "Down", 255, 0, 255);
							Laser(7024, "South", "Down", 0, 255, 255);
							Laser(7025, "South", "Down", 255, 255, 0);
							Laser(7026, "South", "Down", 255, 255, 255);

							Laser(7030, "West", "Left", 0, 0, 255);
							Laser(7031, "West", "Left", 255, 0, 0);
							Laser(7032, "West", "Left", 0, 255, 0);
							Laser(7033, "West", "Left", 255, 0, 255);
							Laser(7034, "West", "Left", 0, 255, 255);
							Laser(7035, "West", "Left", 255, 255, 0);
							Laser(7036, "West", "Left", 255, 255, 255);

							// Break-Away
						case 9000:
						case 9001:
						case 9002:
						case 9003:
						case 9004:
						case 9005:
						case 9006:
						case 9007:
						case 9008:
						case 9009:
						case 9010:
						case 9011:
						case 9012:
							/*
							PE_AddPic "0-9000","Dark Breakaway Block",PicBreakBlock,50,50,50
							PE_AddPic "0-9001","LightBlue Block",PicBreakBlock,200,200,255
							PE_AddPic "0-9002","Red Breakaway Block",PicBreakBlock,255,0,0
							PE_AddPic "0-9003","Green Breakaway Block",PicBreakBlock,0,170,0
							PE_AddPic "0-9004","Pink Breakaway Block",PicBreakBlock,255,200,200
							PE_AddPic "0-9005","Orange Breakaway Block",PicBreakBlock,255,187,3
							PE_AddPic "0-9006","Blue Breakaway Block",PicBreakBlock,0,0,150
							PE_AddPic "0-9007","Brown Breakaway Block",PicBreakBlock,147,74,32
							PE_AddPic "0-9008","Lightgreen Breakaway Block",PicBreakBlock,180,255,180
							PE_AddPic "0-9009","Cyan Breakaway Block",PicBreakBlock,0,255,255
							PE_AddPic "0-9010","Purple Breakaway Block",PicBreakBlock,255,0,255
							PE_AddPic "0-9011","White Breakaway Block",PicBreakBlock,255,255,255
							PE_AddPic "0-9012","Yellow Breakaway Block",PicBreakBlock,255,255,0
							*/
							for (uint32 i = 9000; i <= 9012; ++i) TM->Tex(i)->TexFile = Import(Out, "BreakBlock.png");
							TM->Tex(9000)->Col(50, 50, 50);
							TM->Tex(9001)->Col(200, 200, 255);
							TM->Tex(9002)->Col(200, 0, 0);
							TM->Tex(9003)->Col(0, 170, 0);
							TM->Tex(9004)->Col(255, 200, 200);
							TM->Tex(9005)->Col(255, 187, 3);
							TM->Tex(9006)->Col(0, 0, 150);
							TM->Tex(9007)->Col(147, 74, 32);
							TM->Tex(9008)->Col(180, 255, 180);
							TM->Tex(9009)->Col(0, 255, 255);
							TM->Tex(9010)->Col(180, 0, 255);
							TM->Tex(9011)->Col(255, 255, 255);
							TM->Tex(9012)->Col(255, 255, 0);
							TR->Layers["BREAK"]->Field->Value(x, y, cpz->F[Lay][x][y]);
							break;
						default:
							printf("\x1b[31mError!\x1b[0m\tValue %04d(%04x) not understood in wall layer on coords (%02d,%02d)\n", cpz->F[Lay][x][y], cpz->F[Lay][x][y], x, y);
							return;
						}
						break;
						// Directions and obj
					case 1:
						switch (cpz->F[Lay][x][y]) {
						case 0: break;
							// Ball
						case 1001:
						case 1002:
						case 1003:
						case 1004: {
							auto ball = TR->AddObject(x, y, Ball);
							Import(Out, "Ball.png");
							ball->Data["Direction"] = w[cpz->F[Lay][x][y] - 1001];
						} break;
							// Red Ball
						case 1005:
						case 1006:
						case 1007:
						case 1009: {
							auto ball = TR->AddObject(x, y, RedBall);
							auto d{ cpz->F[Lay][x][y] }; if (d == 1009) d = 1008; // (Correction. It seems there was a little fault here in the original game)
							Import(Out, "Ball.png");
							ball->Data["Direction"] = w[d - 1005];
						} break;
							// Green Ball
						case 1010:
						case 1011:
						case 1012:
						case 1013: {
							auto ball = TR->AddObject(x, y, GreenBall);
							auto d{ cpz->F[Lay][x][y] };
							Import(Out, "Ball.png");
							ball->Data["Direction"] = w[d - 1010];
						} break;
							// Ghost (red)
							MGhost(2000, "North", 255, 0, 0);
							MGhost(2001, "South", 255, 0, 0);
							MGhost(2002, "West", 255, 0, 0);
							MGhost(2003, "East", 255, 0, 0);
							// Ghost (green)
							MGhost(2004, "North", 0, 255, 0);
							MGhost(2005, "South", 0, 255, 0);
							MGhost(2006, "West", 0, 255, 0);
							MGhost(2007, "East", 0, 255, 0);
							// Ghost (blue)
							MGhost(2008, "North", 0, 0, 255);
							MGhost(2009, "South", 0, 0, 255);
							MGhost(2010, "West", 0, 0, 255);
							MGhost(2011, "East", 0, 0, 255);
							// Ghost (yellow)
							MGhost(2012, "North", 255, 255, 0);
							MGhost(2013, "South", 255, 255, 0);
							MGhost(2014, "West", 255, 255, 0);
							MGhost(2015, "East", 255, 255, 0);
							// Ghost (cyan)
							MGhost(2016, "North", 0, 255, 255);
							MGhost(2017, "South", 0, 255, 255);
							MGhost(2018, "West", 0, 255, 255);
							MGhost(2019, "East", 0, 255, 255);
							// Ghost (magenta)
							MGhost(2020, "North", 255, 0, 255);
							MGhost(2021, "South", 255, 0, 255);
							MGhost(2022, "West", 255, 0, 255);
							MGhost(2023, "East", 255, 0, 255);
							// Robot
						case 2100: {
							Import(Out, "Robot.png");
							auto _Robot = TR->AddObject(x, y, Droid);
							_Robot->Data["Wind"] = "North";
						} break;
						case 2101: {
							Import(Out, "Robot.png");
							auto _Robot = TR->AddObject(x, y, Droid);
							_Robot->Data["Wind"] = "South";
						} break;
						case 2102: {
							Import(Out, "Robot.png");
							auto _Robot = TR->AddObject(x, y, Droid);
							_Robot->Data["Wind"] = "West";
						} break;
						case 2103: {
							Import(Out, "Robot.png");
							auto _Robot = TR->AddObject(x, y, Droid);
							_Robot->Data["Wind"] = "East";
						} break;


							// Arrow
						case 3001:
							TM->Tex(arrowup)->TexFile = Import(Out,"Arrow Up.png");
							TR->Layers["DIRECTIONS"]->Field->Value(x, y, arrowup);
							break;
						case 3002:
							TM->Tex(arrowdown)->TexFile = Import(Out, "Arrow Down.png");
							TR->Layers["DIRECTIONS"]->Field->Value(x, y, arrowdown);
							break;
						case 3003:
							TM->Tex(arrowup)->TexFile = Import(Out, "Arrow Left.png");
							TR->Layers["DIRECTIONS"]->Field->Value(x, y, arrowleft);
							break;
						case 3004:
							TM->Tex(arrowup)->TexFile = Import(Out, "Arrow Right.png");
							TR->Layers["DIRECTIONS"]->Field->Value(x, y, arrowright);
							break;
							// Droid Arrows
						case 3011:
							TR->Layers["DIRECTIONS"]->Field->Value(x, y, droidarrownorth);
							break;
						case 3012:
							TR->Layers["DIRECTIONS"]->Field->Value(x, y, droidarrowsouth);
							break;
						case 3013:
							TR->Layers["DIRECTIONS"]->Field->Value(x, y, droidarrowwest);
							break;
						case 3014:
							TR->Layers["DIRECTIONS"]->Field->Value(x, y, droidarroweast);
							break;



							// Trigger
							LaserPlate(7000, 0, 0, 255);
							LaserPlate(7001, 255, 0, 0);
							LaserPlate(7002, 0, 255, 0);
							LaserPlate(7003, 255, 0, 255);
							LaserPlate(7004, 0, 255, 255);
							LaserPlate(7005, 255, 255, 0);
							LaserPlate(7006, 255, 255, 255);
							// Bon
						case 4000:
						case 4001:
						case 4002:
						case 4003:
						case 4004:
						case 4005:
						case 4006:
						case 4007:
						case 4008:
						case 4009:
						case 4010:
						case 4011:
						case 4012:
							TM->Tex(dot)->TexFile = Import(Out, "Dot.png");
							TM->Tex(dot)->b = 0;
							TR->Layers["DIRECTIONS"]->Field->Value(x, y, dot);
							break;

						case 8000:
							TM->Tex(8000)->TexFile = Import(Out, "Bom.jpbf");
							TM->Tex(8000)->AnimSpeed = 4;
							TR->Layers["BOMBS"]->Field->Value(x, y, 8000);
							break;
							// Level plate
						case 9000:
							TM->Tex(levelplate1)->TexFile = Import(Out, "Plate1.png");
							TM->Tex(levelplate1)->b = 0;
							TR->Layers["DIRECTIONS"]->Field->Value(x, y, levelplate1);
						case 9001:
							TM->Tex(levelplate2)->TexFile = Import(Out, "Plate2.png");
							TM->Tex(levelplate2)->b = 0;
							TR->Layers["DIRECTIONS"]->Field->Value(x, y, levelplate2);
							// Irreplacable plates
						case 9002:
							TM->Tex(irreplacableplate1)->TexFile = Import(Out, "Plate1.png");
							TM->Tex(irreplacableplate1)->r = 0;
							TR->Layers["DIRECTIONS"]->Field->Value(x, y, irreplacableplate1);
							break;
						case 9003:
							TM->Tex(irreplacableplate2)->TexFile = Import(Out, "Plate2.png");
							TM->Tex(irreplacableplate2)->r = 0;
							TR->Layers["DIRECTIONS"]->Field->Value(x, y, irreplacableplate2);
							break;
							// Nobuild!
						case 9998:
							TM->Tex(redexit)->TexFile = Import(Out, "Exit.png");
							TM->Tex(redexit)->r = 255;
							TM->Tex(redexit)->g = 0;
							TM->Tex(redexit)->b = 0;
							TR->Layers["DIRECTIONS"]->Field->Value(x, y, redexit);
							cout << "\x1b[31mRed Exit\x1b[0m at (" << (int)x << "," << (int)y << ")\n";
							break;
						case 9997:
							TM->Tex(greenexit)->TexFile = Import(Out, "Exit.png");
							TM->Tex(greenexit)->r = 0;
							TM->Tex(greenexit)->g = 255;
							TM->Tex(greenexit)->b = 0;
							TR->Layers["DIRECTIONS"]->Field->Value(x, y, greenexit);
							cout << "\x1b[32mGreen Exit\x1b[0m at (" << (int)x << "," << (int)y << ")\n";
							break;
						case 9999:
							//TM->Tex(0x24)->TexFile = Import(Out, "NoBuild.png");
							//TR->Layers["DIRECTIONS"]->Field->Value(x, y, nobuild);
							TM->Tex(normalexit)->TexFile = Import(Out, "Exit.png");
							TR->Layers["DIRECTIONS"]->Field->Value(x, y, normalexit);
							cout << "Normal Exit\n";
							break;
						default:
							printf("\x1b[31mError!\x1b[0m\tValue %04d(%04x) not understood in direction/object layer on coords (%02d,%02d)\n", cpz->F[Lay][x][y], cpz->F[Lay][x][y], x, y);
							return;
						}
						break;
						// Floor
					case 2:
						switch (cpz->F[Lay][x][y]) {
						case 0: 
							TR->Layers["DEATH"]->Field->Value(x, y, 1);
							break;
						case 1000:
						case 1001:
						case 1002:
						case 1003: {
							auto duizend{ 1000 + cpz->F[Lay][x][y] };
							TR->Layers["FLOOR"]->Field->Value(x, y, duizend);
							char vloer[200];
							sprintf_s(vloer, "Vloer%d.png", cpz->F[Lay][x][y] - 999);
							TM->Tex(duizend)->TexFile = Import(Out, vloer);
							break;
						}
						case 2000:
							TM->Tex(2004)->TexFile = Import(Out, "Marble.png");
							TR->Layers["FLOOR"]->Field->Value(x, y, 2004);
							break;
						case 2001:
							TM->Tex(2005)->TexFile = Import(Out, "Flat.png");
							TM->Tex(2005)->r = 0;
							TM->Tex(2005)->g = 0;
							TR->Layers["FLOOR"]->Field->Value(x, y, 2005);
							break;
						case 9997:
							TR->Layers["FLOOR"]->Field->Value(x, y, 0);
							// Invisible floor, and no the ball shouldn't be destroyed as the 'Death' floor isn't set here!
							break;
						 // Nobuild!
						case 9999:
							TM->Tex(0x24)->TexFile = Import(Out, "NoBuild.png");
							if (!TR->Layers["DIRECTIONS"]->Field->Value(x, y)) TR->Layers["DIRECTIONS"]->Field->Value(x, y, nobuild);
							TR->Layers["FLOOR"]->Field->Value(x, y, nobuild);
							break;

						default:
							printf("\x1b[31mError!\x1b[0m\tValue %04d(%04x) not understood in floor layer on coords (%02d,%02d)\n", cpz->F[Lay][x][y], cpz->F[Lay][x][y], x, y);
							return;
						}
						break;
					}
				}
			}
		}
		TM->Tex(normalexit)->TexFile = Import(Out, "Exit.png"); // Make sure "Exit" is always set (important for Break-Free puzzles).
		char outf[300]; 
		char outp[300];
		sprintf_s(outf, "Puzzles/Puz%02d", puz + 1);
		sprintf_s(outp, "Puz%02d", puz + 1);
		cout << "Adding to resource as: " << outf << endl;
		TeddySave(TM, Out, outf);
		PID.Value("Puzzles", "Max", to_string(puz + 1));
		PID.Value("Puzzles", outp, cpz->Name);
	}


	void Fractals(JT_Create* Out) {
		char ff[300];
		char tf[300];
		for (byte f = 0; f <= 9; ++f) {
			sprintf_s(ff, "%sFractal%d.png", IncBin.c_str(), f);
			sprintf_s(tf, "Backgrounds/Fractal%d.png", f);
			printf("Fractal %d/9\n", f);
			if (!FileExists(ff)) cout << "\x1b[31mERROR!>\x1b[0m File '" << ff << "' was not found on this system\n";
			Out->AddFile(ff, tf);
		}
	}

	void SaveMeta(JT_Create* Out) {
		const char Lic[200]{ "Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License." };
		cout << "Saving metadata\n";
		PID.Value("Meta", "Name", "BallPlay Genius");
		PID.Value("Meta", "Author", "Jeroen P. Broks");
		PID.Value("Meta", "Created", "2013");
		PID.Value("Tech", "Original_Programming_Language", "BlitzMax");
		PID.Value("Copyright", "License", Lic);
		PID.Value("Copyright", "Copyright", "(c) Jeroen P. Broks 2013");
		PID.Value("Game", "Puzzles", "50");
		Out->AddString("Meta.ini", PID.UnParse());
	}

}
int main(int countargs, char** args) {
	using namespace BPG_Import;

	cout << "Converter for BallPlay Genius\n";
	init_JCR6();
	init_jxsda();
	auto pack = PE_Load("E:/Projects/Applications/VisualStudio/VC/BallPlay++/ImportLevels/Import BallPlay Genius/Original/IncBin/Puzzles/Default.BPGPP");

	cout << "Creating: " << BPG_Import::OutFile << endl;
	JT_Create Out{ BPG_Import::OutFile };
	Fractals(&Out);
	for (byte f = 0; f < 50; ++f) Convert(&Out, pack, f);
	SaveMeta(&Out);
	cout << "Aliasing stuff!\n";
	Out.Alias("Textures/Plate1.png", "Tools/Plate1.png");
	Out.Alias("Textures/Plate2.png", "Tools/Plate2.png");
	Out.Alias("Textures/Muur1.png", "Tools/Barrier.png");
	Out.Alias("Textures/Ball.png", "Objects/Ball.png");
	cout << "Adding trashcan as Tools/Remove.png\n";
	Out.AddFile(ExtractDir(args[0])+"/Trashcan.png", "Tools/Remove.png");
	Out.Close();
	cout << "All done!";
}