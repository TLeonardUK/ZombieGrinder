using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;

namespace GameMonitor.Utils
{
    /// <summary>
    /// CustomClass (Which is binding to property grid)
    /// </summary>
    [TypeConverter(typeof(ExpandableObjectConverter))]
    public class CustomPropertyGridObject : ICustomTypeDescriptor
    {
        List<CustomProperty> Properties = new List<CustomProperty>();

        public void Add(CustomProperty Value)
        {
            Properties.Add(Value);
        }

        public void Remove(string Name)
        {
            foreach (CustomProperty prop in Properties)
            {
                if (prop.Name == Name)
                {
                    Properties.Remove(prop);
                    return;
                }
            }
        }

        public CustomProperty this[int index]
        {
            get
            {
                return (CustomProperty)Properties[index];
            }
            set
            {
                Properties[index] = (CustomProperty)value;
            }
        }

        #region "TypeDescriptor Implementation"]
        public override string ToString()
        {
            return "";
        }

        public String GetClassName()
        {
            return TypeDescriptor.GetClassName(this, true);
        }

        public AttributeCollection GetAttributes()
        {
            return TypeDescriptor.GetAttributes(this, true);
        }

        public String GetComponentName()
        {
            return TypeDescriptor.GetComponentName(this, true);
        }

        public TypeConverter GetConverter()
        {
            return TypeDescriptor.GetConverter(this, true);
        }

        public EventDescriptor GetDefaultEvent()
        {
            return TypeDescriptor.GetDefaultEvent(this, true);
        }

        public PropertyDescriptor GetDefaultProperty()
        {
            return TypeDescriptor.GetDefaultProperty(this, true);
        }

        public object GetEditor(Type editorBaseType)
        {
            return TypeDescriptor.GetEditor(this, editorBaseType, true);
        }

        public EventDescriptorCollection GetEvents(Attribute[] attributes)
        {
            return TypeDescriptor.GetEvents(this, attributes, true);
        }

        public EventDescriptorCollection GetEvents()
        {
            return TypeDescriptor.GetEvents(this, true);
        }

        public PropertyDescriptorCollection GetProperties(Attribute[] attributes)
        {
            PropertyDescriptor[] newProps = new PropertyDescriptor[this.Properties.Count];
            for (int i = 0; i < this.Properties.Count; i++)
            {
                CustomProperty prop = (CustomProperty)this[i];
                newProps[i] = new CustomPropertyDescriptor(ref prop, attributes);
            }

            return new PropertyDescriptorCollection(newProps);
        }

        public PropertyDescriptorCollection GetProperties()
        {
            return TypeDescriptor.GetProperties(this, true);
        }

        public object GetPropertyOwner(PropertyDescriptor pd)
        {
            return this;
        }
        #endregion
    }

    public delegate void CustomPropertyChangedEventHandler(CustomProperty Prop);

    public class CustomProperty
    {
        private string sName = string.Empty;
        private bool bReadOnly = false;
        private bool bVisible = true;
        private object objValue = null;
        private string sFullPath = "";

        public CustomProperty(string sName, object value, Type type, bool bReadOnly, bool bVisible, string fullPath)
        {
            this.sName = sName;
            this.objValue = value;
            this.type = type;
            this.bReadOnly = bReadOnly;
            this.bVisible = bVisible;
            this.sFullPath = fullPath;
        }

        public event CustomPropertyChangedEventHandler Changed;

        private Type type;
        public Type Type
        {
            get { return type; }
        }

        public bool ReadOnly
        {
            get
            {
                return bReadOnly;
            }
        }

        public string Name
        {
            get
            {
                return sName;
            }
        }

        public string FullPath
        {
            get
            {
                return sFullPath;
            }
        }

        public bool Visible
        {
            get
            {
                return bVisible;
            }
        }

        public object Value
        {
            get
            {
                return objValue;
            }
            set
            {
                objValue = value;
                if (Changed != null)
                {
                    Changed(this);
                }
            }
        }
    }

    public class CustomPropertyDescriptor : PropertyDescriptor
    {
        CustomProperty m_Property;
        public CustomPropertyDescriptor(ref CustomProperty myProperty, Attribute[] attrs)
            : base(myProperty.Name, attrs)
        {
            m_Property = myProperty;
        }

        #region PropertyDescriptor specific

        public override AttributeCollection Attributes
        {
            get
            {
                return new AttributeCollection(null);
            }
        }


        public override bool CanResetValue(object component)
        {
            return false;
        }

        public override Type ComponentType
        {
            get { return null; }
        }

        public override object GetValue(object component)
        {
            return m_Property.Value;
        }

        public override string Description
        {
            get { return m_Property.Name; }
        }

        public override string Category
        {
            get { return "Settings"; }
        }

        public override string DisplayName
        {
            get { return m_Property.Name; }
        }

        public override bool IsReadOnly
        {
            get { return m_Property.ReadOnly; }
        }

        public override bool IsBrowsable
        {
            get { return true; }
        }

        public override void ResetValue(object component)
        {
            //Have to implement
        }

        public override bool ShouldSerializeValue(object component)
        {
            return false;
        }

        public override void SetValue(object component, object value)
        {
            m_Property.Value = value;
        }

        public override Type PropertyType
        {
            get { return m_Property.Type; }
        }

        #endregion
    }
}
