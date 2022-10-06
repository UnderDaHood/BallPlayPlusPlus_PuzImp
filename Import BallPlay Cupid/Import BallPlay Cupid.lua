-- <License Block>
-- BallPlay++
-- BallPlay Cupid Puzzle Importer (Script)
-- 
-- 
-- 
-- (c) Jeroen P. Broks, 2022
-- 
-- This program is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
-- 
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
-- You should have received a copy of the GNU General Public License
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.
-- 
-- Please note that some references to data like pictures or audio, do not automatically
-- fall under this licenses. Mostly this is noted in the respective files.
-- 
-- Version: 22.09.28
-- </License Block>
-- Script
local load=load
local sprintf=string.format
local TLay={walls="WALL",floors="FLOOR"}
local Missions={Normal="Normal",[1]="Normal",["Break-Away"]="Break Away",[2]="Break Away",["Break-Free"]="Break Free",Walkthrough="Walkthrough",["Color Split"]="Color Split", Collect="Dot Collector", ["Break&Collect"]="Break & Collect"}
local Tools={plate1={"Plate1","Plate/"},plate2={"Plate2","Plate\\"},barrier={"Barrier"},trash={"Remove"}}

local DirTypes = {
			redplate1 = 0x20,
			userplate1 = 0x20,
			redplate2 = 0x21,
			userplate2 = 0x21,
			irreplacableplate1 = 0x22,
			irreplacableplate2 = 0x23,
			nobuild = 0x24,
			arrowwest = 0x25,
			arrowleft = 0x25,
			arroweast = 0x26,
			arrowright = 0x26,
			arrowdown = 0x27,
			arrowsouth = 0x27,
			arrowup = 0x28,
			arrownorth = 0x28,
			levelplate1 = 0x29, -- Same as user plate, but in the puzzle by default, and user plate was placed by the player
			levelplate2 = 0x2a,
			droidarrowwest = 0x2b,
			droidarrowleft = 0x2b,
			droidarroweast = 0x2c,
			droidarrowright = 0x2c,
			droidarrowdown = 0x2d,
			droidarrowsouth = 0x2d,
			droidarrowup = 0x2e,
			droidarrownorth = 0x2e,
			dot = 0x31,
			normalexit = 0x70,
			greenexit = 0x71,
			redexit = 0x72, 
			blueexit = 0x73,
			emberexit = 0x74,
			laser = 0xf0,
			lasertrigger = 0xf1,
			Exit = 0x70,
			ExitGreen = 0x71,
			GreenExit = 0x71,
			ExitRed = 0x72,
			RedExit = 0x72,

			GirlHome = 2, girlhome = 2,
			dot=3,Dot=3,

			LaserRedNorth = 0xfff0,
			LaserRedSouth = 0xfff1,
			LaserRedWest = 0xfff2,
			LaserRedEast = 0xfff3,

			LaserBlueNorth = 0xfff4,
			LaserBlueSouth = 0xfff5,
			LaserBlueWest = 0xfff6,
			LaserBlueEast = 0xfff7,

			LaserEmberNorth = 0xfff8,
			LaserEmberSouth = 0xfff9,
			LaserEmberWest = 0xfffa,
			LaserEmberEast = 0xfffb,

			LaserGreenNorth = 0xfffc,
			LaserGreenSouth = 0xfffd,
			LaserGreenWest = 0xfffe,
			LaserGreenEast = 0xffff,

			LaserPlateRed = 0xfef0,
			LaserPlateBlue = 0xfef4,
			LaserPlateEmber = 0xfef8,
			LaserPlateGreen = 0xfefc

		}

local ObjTypes = {
			Ball = 0x01,
			NormalBall = 0x01,
			BallNormal = 0x01,
			BallGreen = 0x02,
			GreenBall = 0x02,
			BallRed = 0x03,
			RedBall = 0x03,
			Ghost = 0x04, --//0x12,
			Droid = 0x05, --//0x11		
			Girl = 0x06,			
			BlueBall = 0x07,
			EmberBall = 0x08
		}

local DConv={
	zplate1="irreplacableplate1",
	zplate2="irreplacableplate2",
	a_exit="normalexit",
	gplate1="levelplate1",
	gplate2="levelplate2",
	womanhome="GirlHome",
	cr_exit="redexit",
	cg_exit="greenexit",
	cb_exit="blueexit",
	ce_exit="emberexit",
	zzarrow_droid_up="droidarrownorth",
	zzarrow_droid_down="droidarrowsouth",
	zzarrow_droid_left="droidarrowwest",
	zzarrow_droid_right="droidarroweast",
	zzarrow_normal_right="arroweast",
	zzarrow_normal_down="arrowsouth",
	zzarrow_normal_left="arrowwest",
	zzarrow_normal_up="arrownorth",
	dot="dot"
};
local breakblocks = { 
	bb01 = {"breakblock",  0,180,255},
	bb02 = {"breakblock",255,  0,  0},
	bb03 = {"breakblock",  0,180,  0},
	bb04 = {"breakblock",255,180,180},
	bb05 = {"breakblock",180,100,  0},
	bb06 = {"breakblock",  0,  0,180},
	bb07 = {"breakblock",100, 60,  0},
	bb08 = {"breakblock",180,255,  0},
	bb09 = {"breakblock",  0,255,180},
	bb10 = {"breakblock",180,  0,255},
	bb11 = {"breakblock",255,255,255},
	bb12 = {"breakblock",255,180,  0}
}

local substr = string.sub
local function Mid(s,o,l)
	local ln
	local of
	local st
	ln=l or 1
	of=o or 1
	st=s or ""
	return substr(st,of,(of+ln)-1)
end

local function Left(s, l) 
	--if not _Neil.Assert(type(s)=="string","String exected as first argument for 'left'") then return end
	l = l or 1
	--_Neil.Assert(type(l)=="number","Number expected for second argument in 'left'")
	return substr(s,1,l)
end

local function Right(s,l)
			local ln
			local st
			ln = l or 1
			st = s or "nostring"
			return substr(st,-ln,-1)
end

local function Prefixed(str,pref)
	return Left(str,#pref)==pref
end

local function printf(fmt,...) 
	io.write(sprintf(fmt,...))
end

-- Quick Laser Setup
for _,lasercol in ipairs{"Red","Blue","Ember","Green"} do
	for laserdirs,laserdirt in pairs{Down="South",Up="North",Left="West",Right="East"} do
		-- laser_Down_red
		DConv[sprintf("laser_%s_%s",laserdirs,lasercol:lower())] = sprintf("Laser%s%s",lasercol,laserdirt)
	end
end

local function ArT(x,y) return sprintf(">%d:%d<",x,y) end

function MainData(lvnum,key,value)
	_MainData(lvnum,key,sprintf("%s",value))
end

local objswitch
local D2W={D="South",U="North",L="West",R="East"}
function Convert(lvnum)
	printf("Converting level: %02d\n",lvnum)
	local suc,script = GetPuzzle(lvnum)
	if (not suc) then return end
	local cScript,Err=load(script); assert(cScript,sprintf("Failed to load the script\n-> %s",Err));
	local Data,xData=cScript()
	SetName(lvnum,Data["title"]);
	if Data.format[1]~=25 or Data.format[2]~=15 then
		printf("WARNING! Format is not 25x15 but it's %02x%02d in stead'",Data.format[1],Data.format[2])
	end
	for y=0,Data.format[2] do
		for x=0,Data.format[1] do
			for io,it in pairs(TLay) do
				if Data[io][ArT(x,y)] then TexLayModify(lvnum,it,x,y,Data[io][ArT(x,y)]) end
			end
			if Data.obstacles[ArT(x,y)] then
				local obst=Data.obstacles[ArT(x,y)]
				if (Left(obst,2)=="bb") then
					--NonFatal(sprintf("BreakBlock not yet implemented (%s)\n",obst))
					local bb=breakblocks[obst];
					assert(bb,sprintf("No data for breakblock: %s",obst))
					printf("Break Block: %s on (%02d,%02d)   %02x%02x%02x -- ",obst,x,y,bb[2],bb[3],bb[4])
					BreakBlock(lvnum,x,y,bb[2],bb[3],bb[4])
				elseif obst=="zzzglass" then
					TexLayModify(lvnum,"GLASS",x,y,"pz_objects_glass")
					DirLayModify(lvnum,x,y,DirTypes.nobuild)
				elseif Prefixed(obst,"platelaser_") then
					--NonFatal("Laser plates not yet been implemented ("..obst..")")
					local delnum = #("platelaser_")
					local pcol=Right(obst,#obst-delnum)
					local col = Left(pcol,1):upper()..Right(pcol,#pcol-1):lower()
					printf("Laser Plate (%2d,%2d): %s",x,y,col)
					DirLayModify(lvnum,x,y,DirTypes["LaserPlate"..col])
				elseif Prefixed(obst,"laser") then
					print(obst)
					Laser(lvnum,x,y,DirTypes[DConv[obst]])
					--NonFatal(sprintf("Laser not yet fully supported (%02d,%02d)",x,y))
				else
					assert(DConv[obst],sprintf("No proper replacement known for obstacle '%s' at (%02d,%02d)",obst,x,y))
					assert(DirTypes[DConv[obst]],sprintf("Enum error '%s'=>'%s' (%02d,%02d)",obst,DConv[obst],x,y))
					DirLayModify(lvnum,x,y,DirTypes[DConv[obst]])
				end
			end
		end

	end
	--local miss
	assert(Missions[Data.mission or Data.missionum],sprintf("Unknown mission '%s'!",Data.mission or Data.missionum));
	MainData(lvnum,"Mission",Missions[Data.mission or Data.missionum])
	for k,v in pairs(Data.tools) do
		assert(Tools[k],"No BP++ equivalent found foor tool '"..k.."'!")
		for _,tk in ipairs(Tools[k]) do
			MainData(lvnum,tk,v)
		end
	end
	MainData(lvnum,"Required",Data.reqballs)
	if (Data.background) then
		MainData(lvnum,"Background","*FUNCTION*");
		MainData(lvnum,"BackgroundFunction",Data.background);
	end

	for oi,o in ipairs(Data.objects) do
		printf("(%02d,%02d)\t",o.x,o.y)
		printf("%03d: %s: ",oi,o.kind)
		--objswitch = objswitch or {
		;(({
			ball = function()
				Object(lvnum,ObjTypes.Ball,o.x,o.y,255,255,255,255,D2W[o.dir])
			end,
			ballember=function()
				Object(lvnum,ObjTypes.EmberBall,o.x,o.y,255,180,0,255,D2W[o.dir])
			end,
			ballblue=function()
				Object(lvnum,ObjTypes.BlueBall,o.x,o.y,0,0,255,255,D2W[o.dir])
			end,
			ballred=function()
				Object(lvnum,ObjTypes.RedBall,o.x,o.y,255,0,0,255,D2W[o.dir])
			end,
			ballgreen=function()
				Object(lvnum,ObjTypes.GreenBall,o.x,o.y,0,255,0,255,D2W[o.dir])
			end,

			ghostamber=function()
				Object(lvnum,ObjTypes.Ghost,o.x,o.y,255,180,0,180,D2W[o.dir])
			end,
			ghostred=function()
				Object(lvnum,ObjTypes.Ghost,o.x,o.y,255,0,0,180,D2W[o.dir])
			end,
			ghostblue=function()
				Object(lvnum,ObjTypes.Ghost,o.x,o.y,0,180,255,180,D2W[o.dir])
			end,
			ghostgreen=function()
				Object(lvnum,ObjTypes.Ghost,o.x,o.y,0,255,0,180,D2W[o.dir])
			end,
			woman=function()
				Object(lvnum,ObjTypes.Girl,o.x,o.y)
			end,
			robot=function()
				Object(lvnum,ObjTypes.Droid,o.x,o.y,255,255,255,255,D2W[o.dir])
			end
		})[o.kind] or function() error("I do not know how to handle object type: "..o.kind) end)()
		--assert(objswitch[o.kind],"I do not know how to handle object type: "..o.kind)
		--objswitch[o.kind]()
	end
end