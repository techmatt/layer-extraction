using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Engine;
using System.IO;

namespace BaseCodeApp
{
    class PaletteCache
    {
        String baseDir = "";

        public PaletteCache(String dir)
        {
            baseDir = dir;
            Directory.CreateDirectory(baseDir);
        }
        private String GetKey(String method, int k, String image)
        {
            String imageBasename = new FileInfo(image).Name;
            return Path.Combine(baseDir, method + "-" + k + "-" + Util.ConvertFileName(imageBasename, "", ".txt"));
        }

        public void SavePalette(String method, int k, String image, PaletteData data)
        {   
            File.WriteAllText(GetKey(method, k, image), data.ToString());
        }

        public bool Exists(String method, int k, String image)
        {
            return File.Exists(GetKey(method,k,image));
        }

        public PaletteData GetPalette(String method, int k, String image)
        {
            String line = File.ReadAllText(GetKey(method,k,image));
            return PaletteData.FromString(line);
        }


    }
}
