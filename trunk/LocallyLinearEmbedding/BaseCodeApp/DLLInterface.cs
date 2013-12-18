using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.IO;
using System.Net;
using MathNet.Numerics.LinearAlgebra.Double;
using Engine;
using System.Linq;
using System.Diagnostics;
using System.Windows.Input;

namespace BaseCodeApp
{
    [StructLayout(LayoutKind.Sequential), Serializable]
    public struct BCBitmapInfo
    {
        [MarshalAs(UnmanagedType.U4)]
        public int width;
        [MarshalAs(UnmanagedType.U4)]
        public int height;
        [MarshalAs(UnmanagedType.SysInt)]
        public IntPtr colorData;
    }

    [StructLayout(LayoutKind.Sequential), Serializable]
    public struct BCLayerInfo
    {
        [MarshalAs(UnmanagedType.U4)]
        public int width;
        [MarshalAs(UnmanagedType.U4)]
        public int height;
        [MarshalAs(UnmanagedType.R8)]
        public double d0;
        [MarshalAs(UnmanagedType.R8)]
        public double d1;
        [MarshalAs(UnmanagedType.R8)]
        public double d2;
        [MarshalAs(UnmanagedType.SysInt)]
        public IntPtr weights;
    }

    [StructLayout(LayoutKind.Sequential), Serializable]
    public struct BCLayers
    {
        [MarshalAs(UnmanagedType.U4)]
        public int numLayers;
        [MarshalAs(UnmanagedType.SysInt)]
        public IntPtr layers;
    }

    public class DLLInterface
    {
        const string BaseCodeDLL = "BaseCode.dll";
        [DllImport(BaseCodeDLL)]
        public static extern IntPtr BCInit();
        [DllImport(BaseCodeDLL)]
        private static extern UInt32 BCProcessCommand(IntPtr context, [In, MarshalAs(UnmanagedType.LPStr)] String command);
        [DllImport(BaseCodeDLL)]
        public static extern IntPtr BCQueryBitmapByName(IntPtr context, [In, MarshalAs(UnmanagedType.LPStr)] String bitmapName);
        [DllImport(BaseCodeDLL)]
        public static extern IntPtr BCQueryStringByName(IntPtr context, [In, MarshalAs(UnmanagedType.LPStr)] String stringName);
        [DllImport(BaseCodeDLL)]
        public static extern Int32 BCQueryIntegerByName(IntPtr context, [In, MarshalAs(UnmanagedType.LPStr)] String integerName);
        [DllImport(BaseCodeDLL)]
        public static extern IntPtr BCExtractLayers(IntPtr context, BCBitmapInfo bitmap, IntPtr palette, [In, MarshalAs(UnmanagedType.I4)]int paletteSize, [In, MarshalAs(UnmanagedType.LPStr)] String layerConstraints, [In, MarshalAs(UnmanagedType.Bool)] bool autoCorrect, [In, MarshalAs(UnmanagedType.LPStr)] String imageFile);
        [DllImport(BaseCodeDLL)]
        public static extern IntPtr BCSegmentImage(IntPtr context, BCBitmapInfo bitmap);
        [DllImport(BaseCodeDLL)]
        public static extern IntPtr BCSynthesizeLayers(IntPtr context);
        [DllImport(BaseCodeDLL)]
        public static extern IntPtr BCOutputMesh(IntPtr context, BCBitmapInfo bitmap, IntPtr palette, [In, MarshalAs(UnmanagedType.I4)]int paletteSize, [In, MarshalAs(UnmanagedType.LPStr)] String filename);
        [DllImport(BaseCodeDLL)]
        public static extern IntPtr BCLoadVideo(IntPtr context, [In, MarshalAs(UnmanagedType.LPStr)] String filename, [In, MarshalAs(UnmanagedType.I4)]int paletteSize);
        [DllImport(BaseCodeDLL)]
        public static extern Int32 BCGetVideoPaletteSize(IntPtr context);
        [DllImport(BaseCodeDLL)]
        public static extern Byte BCGetVideoPalette(IntPtr context, [In, MarshalAs(UnmanagedType.I4)]int paletteindex, [In, MarshalAs(UnmanagedType.I4)]int index);
        [DllImport(BaseCodeDLL)]
        public static extern IntPtr BCSetVideoPalette(IntPtr context, [In, MarshalAs(UnmanagedType.I4)]int paletteindex, byte r, byte g, byte b);
        [DllImport(BaseCodeDLL)]
        public static extern Byte BCGetOriginalVideoPalette(IntPtr context, [In, MarshalAs(UnmanagedType.I4)]int paletteindex, [In, MarshalAs(UnmanagedType.I4)]int index);
        [DllImport(BaseCodeDLL)]
        public static extern IntPtr BCSaveVideoFrames(IntPtr context);
        [DllImport(BaseCodeDLL)]
        public static extern IntPtr BCSaveVideoPaletteImage(IntPtr context);
        [DllImport(BaseCodeDLL)]
        public static extern IntPtr BCSetVideoPreviewLayerIndex(IntPtr context, [In, MarshalAs(UnmanagedType.I4)]int index);
        [DllImport(BaseCodeDLL)]
        public static extern Int32 BCGetVideoHeight(IntPtr context);
        [DllImport(BaseCodeDLL)]
        public static extern Int32 BCGetVideoWidth(IntPtr context);

        public IntPtr baseCodeDLLContext = (IntPtr)0;

        public DLLInterface()
        {
            if (baseCodeDLLContext == (IntPtr)0)
            {
                baseCodeDLLContext = BCInit();
            }
        }

        public UInt32 ProcessCommand(String command)
        {
            return BCProcessCommand(baseCodeDLLContext, command);
        }

        public Bitmap GetBitmap(String bitmapName)
        {
            IntPtr bitmapInfoUnmanaged = BCQueryBitmapByName(baseCodeDLLContext, bitmapName);
            if (bitmapInfoUnmanaged == (IntPtr)0) return null;

            BCBitmapInfo bitmapInfo = (BCBitmapInfo)Marshal.PtrToStructure(bitmapInfoUnmanaged, typeof(BCBitmapInfo));

            return new Bitmap(bitmapInfo.width, bitmapInfo.height, bitmapInfo.width * 4, System.Drawing.Imaging.PixelFormat.Format32bppRgb, bitmapInfo.colorData);
        }

        public String GetString(String stringName)
        {
            IntPtr stringPtr = BCQueryStringByName(baseCodeDLLContext, stringName);
            if (stringPtr == (IntPtr)0)
            {
                return null;
            }
            else
            {
                return Marshal.PtrToStringAnsi(stringPtr);
            }
        }

        public void LoadVideo(String filename, int paletteSize)
        {
            BCLoadVideo(baseCodeDLLContext, filename, paletteSize);
        }

        public int GetVideoPaletteSize()
        {
            return BCGetVideoPaletteSize(baseCodeDLLContext);
        }

        public byte GetVideoPalette(int paletteindex, int index)
        {
            return BCGetVideoPalette(baseCodeDLLContext, paletteindex, index);
        }

        public void SetVideoPalette(int paletteindex, byte r, byte g, byte b)
        {
            BCSetVideoPalette(baseCodeDLLContext, paletteindex, r, g, b);
        }

        public byte GetOriginalVideoPalette(int paletteindex, int index)
        {
            return BCGetOriginalVideoPalette(baseCodeDLLContext, paletteindex, index);
        }

        public void SaveVideoFrames()
        {
            BCSaveVideoFrames(baseCodeDLLContext);
        }

        public void SaveVideoPaletteImage()
        {
            BCSaveVideoPaletteImage(baseCodeDLLContext);
        }

        public void SetVideoPreviewLayerIndex(int index)
        {
            BCSetVideoPreviewLayerIndex(baseCodeDLLContext, index);
        }

        public int GetVideoHeight()
        {
            return BCGetVideoHeight(baseCodeDLLContext);
        }
        public int GetVideoWidth()
        {
            return BCGetVideoWidth(baseCodeDLLContext);
        }
    }
}
