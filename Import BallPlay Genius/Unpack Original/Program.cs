// Lic:
// BallPlay++
// Unpack original stuff
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
// Version: 22.09.14
// EndLic
using UseJCR6;
using TrickyUnits;
void print(params object[] objects) {foreach(var obj in objects) Console.Write(obj.ToString());Console.WriteLine(); }
const string original = "F:/Old_Scyndi/Volume_Itself/CLOSET BACKUPS/USB HD Phantasar/Projects/BallPlay Genius.jcr";
const string target = "E:/Projects/Applications/VisualStudio/VC/BallPlay++/ImportLevels/Import BallPlay Genius/Original/";

print("Init JCR5"); JCR_JCR5.Init();
print("Init zlib"); JCR6_zlib.Init();

print("Reading: ", original);
var j = JCR6.Dir(original);
if (j == null) {
	print("Error! File could not be read: ", JCR6.JERROR);
	return;
}

uint errors = 0;
foreach(var e in j.Entries) {
	var ent=e.Value.Entry;
	var Ok = true;
	var dx = ent.Split('/');
	Ok = Ok && qstr.ExtractExt(dx[0]).ToLower() != "app";
	Ok = Ok && qstr.ExtractExt(ent).ToLower() != "exe";
	if (Ok) {
		var tdir = $"{target}{qstr.ExtractDir(ent)}";
		if (!Directory.Exists(tdir)) {
			print("  Creating:", tdir);
			Directory.CreateDirectory(tdir);
		}
		print("Extracting: ", ent);
		try {
			var buf = j.JCR_B(ent);
			var bout = QuickStream.WriteFile($"{target}{ent}");
			bout.WriteBytes(buf);
			bout.Close();
		} catch (Exception ex) {			
			Console.ForegroundColor = ConsoleColor.Red;
			print($"\tError caught! ({++errors})");
			Console.ResetColor();
			print($"\t\t{ex.Message}");
#if DEBUG
			print($"=====\n{ex.StackTrace}\n====\n\n");
#endif
		}
	} else {
		print("   Skipped: ", ent);
	}
}