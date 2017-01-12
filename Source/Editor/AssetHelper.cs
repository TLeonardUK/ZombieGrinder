using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Xml;

namespace Editor
{
    public enum AssetType
    {
        Atlas,
        AudioBank,
        Font,
        Language,
        Layout,
        Map,
        ParticleFX,
        Raw,
        Script,
        Shader,
        Video,
        UpgradeTreeDB
    }

    public static class AssetHelper
    {
        public static string GetRootXMLTag(string path)
        {
            XmlDataDocument doc = new XmlDataDocument();
            try
            {
                doc.Load(path);
            }
            catch (XmlException)
            {
                return "";
            }

            return doc.DocumentElement.Name;
        }

        public static string GetEditorTypeTag(string path)
        {
            XmlDataDocument doc = new XmlDataDocument();
            try
            {
                doc.Load(path);
            }
            catch (XmlException)
            {
                return "";
            }

            try
            {
                XmlElement editorNode = doc.DocumentElement["editor"];
                XmlElement assetTypeNode = editorNode["asset_type"];

                return assetTypeNode.InnerText;
            }
            catch (NullReferenceException)
            {
                return "raw";
            }
        }

        public static AssetType GetAssetType(string path)
        {
            if (Path.GetExtension(path) == ".xs")
            {
                return AssetType.Script;
            }
            else if (Path.GetExtension(path) == ".ogv")
            {
                return AssetType.Video;
            }
            else if (Path.GetExtension(path) == ".zgmap")
            {
                return AssetType.Map;
            }
            else if (Path.GetExtension(path) == ".xml")
            {
                string tag = GetRootXMLTag(path).ToLower();
                if (tag == "resources")
                {
                    return AssetType.Language;
                }
                else
                {
                    tag = GetEditorTypeTag(path).ToLower();
                    switch (tag)
                    {
                        case "atlas":           return AssetType.Atlas;
                        case "audiobank":       return AssetType.AudioBank;
                        case "font":            return AssetType.Font;
                        case "layout":          return AssetType.Layout;
                        case "map":             return AssetType.Map;
                        case "particlefx":      return AssetType.ParticleFX;
                        case "raw":             return AssetType.Raw;
                        case "script":          return AssetType.Script;
                        case "shader":          return AssetType.Shader;
                        case "video":           return AssetType.Video;
                        case "upgradetreedb":   return AssetType.UpgradeTreeDB;
                    }
                }
            }

            return AssetType.Raw;
        }

        public static string GetAssetTypeName(AssetType type)
        {
            switch (type)
            {
                case AssetType.Atlas:           return "Atlas";
                case AssetType.AudioBank:       return "Audio Bank";
                case AssetType.Font:            return "Font";
                case AssetType.Language:        return "Language";
                case AssetType.Layout:          return "UI Layout";
                case AssetType.Map:             return "Map";
                case AssetType.ParticleFX:      return "Particle FX";
                case AssetType.Raw:             return "Raw";
                case AssetType.Script:          return "Script";
                case AssetType.Shader:          return "Shader";
                case AssetType.Video:           return "Video";
                case AssetType.UpgradeTreeDB:   return "Upgrade Tree DB";
            }
            return "Unknown";
        }
    }
}
