import lldb
import string
import struct

def __lldb_init_module(debugger, internal_dict):
    debugger.HandleCommand('type summary add --expand -F CommonLLDBFormatters.String_SummaryProvider -x "ngine::TString<.+>$"')
    debugger.HandleCommand('type summary add --expand -F CommonLLDBFormatters.StringView_SummaryProvider -x "ngine::TStringView<.+>$"')
    debugger.HandleCommand('type summary add --expand -F CommonLLDBFormatters.MathVector2_SummaryProvider -x "ngine::Math::TVector2<.+>"')
    debugger.HandleCommand('type summary add --expand -F CommonLLDBFormatters.MathVector3_SummaryProvider -x "ngine::Math::TVector3<.+>"')
    debugger.HandleCommand('type summary add --expand -F CommonLLDBFormatters.MathVector4_SummaryProvider -x "ngine::Math::TVector4<.+>"')
    debugger.HandleCommand('type summary add --expand -F CommonLLDBFormatters.MathQuaternion_SummaryProvider -x "ngine::Math::TQuaternion<.+>"')
    debugger.HandleCommand('type summary add --expand -F CommonLLDBFormatters.MathTransform_SummaryProvider -x "ngine::Math::TTransform<.+>"')
    debugger.HandleCommand('type summary add --expand -F CommonLLDBFormatters.MathColor_SummaryProvider -x "ngine::Math::TColor<.+>"')
    debugger.HandleCommand('type summary add --expand -F CommonLLDBFormatters.MathRectangle_SummaryProvider -x "ngine::Math::TRectangle<.+>"')
    debugger.HandleCommand('type summary add --expand -F CommonLLDBFormatters.MathAngle_SummaryProvider -x "ngine::Math::TAngle<.+>"')
    debugger.HandleCommand('type summary add --expand -F CommonLLDBFormatters.MathClampedValue_SummaryProvider -x "ngine::Math::ClampedValuef"')
    debugger.HandleCommand('type summary add --expand -F CommonLLDBFormatters.MathClampedValue_SummaryProvider -x "ngine::Math::ClampedValue<.+>"')
    debugger.HandleCommand('type summary add --expand -F CommonLLDBFormatters.Vector_SummaryProvider -x "ngine::TVector<.+>$"')
    debugger.HandleCommand('type synthetic add -x "ngine::TVector<.*>" --python-class CommonLLDBFormatters.SceneriVectorProvider')
    debugger.HandleCommand('type summary add --expand -F CommonLLDBFormatters.ArrayView_SummaryProvider -x "ngine::ArrayView<.+>$"')
    debugger.HandleCommand('type synthetic add -x "ngine::ArrayView<.*>" --python-class CommonLLDBFormatters.SceneriArrayViewProvider')
    debugger.HandleCommand('type summary add --expand -F CommonLLDBFormatters.Array_SummaryProvider -x "ngine::Array<.+>$"')
    debugger.HandleCommand('type synthetic add -x "ngine::Array<.*>" --python-class CommonLLDBFormatters.SceneriArrayProvider')
    debugger.HandleCommand('type summary add -F CommonLLDBFormatters.Guid_SummaryProvider ngine::Guid')
    debugger.HandleCommand('type summary add -F CommonLLDBFormatters.Guid_SummaryProvider ngine::Asset::Guid')

def String_SummaryProvider(valobj, dict):
    provider = SceneriStringProvider(valobj, dict)
    return "{ length = %d, contents = '%s' }" % (provider.get_length(), provider.to_string())

def StringView_SummaryProvider(valobj, dict):
    provider = SceneriStringViewProvider(valobj, dict)
    return "{ length = %d, contents = '%s' }" % (provider.get_length(), provider.to_string())

def MathVector2_SummaryProvider(valobj, dict):
    return "{ x = %s, y = %s }" % (valobj.GetChildMemberWithName('x').GetValue(), valobj.GetChildMemberWithName('y').GetValue())

def MathVector3_SummaryProvider(valobj, dict):
    return "{ x = %s, y = %s, z = %s }" % (valobj.GetChildMemberWithName('x').GetValue(), valobj.GetChildMemberWithName('y').GetValue(), valobj.GetChildMemberWithName('z').GetValue())

def MathVector4_SummaryProvider(valobj, dict):
    return "{ x = %s, y = %s, z = %s, w = %s }" % (valobj.GetChildMemberWithName('x').GetValue(), valobj.GetChildMemberWithName('y').GetValue(), valobj.GetChildMemberWithName('z').GetValue(), valobj.GetChildMemberWithName('w').GetValue())

def MathQuaternion_SummaryProvider(valobj, dict):
    return "{ x = %s, y = %s, z = %s, w = %s }" % (valobj.GetChildMemberWithName('x').GetValue(), valobj.GetChildMemberWithName('y').GetValue(), valobj.GetChildMemberWithName('z').GetValue(), valobj.GetChildMemberWithName('w').GetValue())

def MathTransform_SummaryProvider(valobj, dict):
    scaledQuaternion = valobj.GetChildMemberWithName("m_rotation");
    position = valobj.GetChildMemberWithName("m_location");
    quaternion = scaledQuaternion.GetChildMemberWithName("m_rotation");
    scale = scaledQuaternion.GetChildMemberWithName("m_scale");
    return "{ { position = x: %s, y: %s, z: %s}, { rotation = x: %s, y: %s, z: %s, w: %s }, { scale = x: %s, y: %s, z: %s } }" % (position.GetChildMemberWithName('x').GetValue(), position.GetChildMemberWithName('y').GetValue(), position.GetChildMemberWithName('z').GetValue(), quaternion.GetChildMemberWithName('x').GetValue(), quaternion.GetChildMemberWithName('y').GetValue(), quaternion.GetChildMemberWithName('z').GetValue(), quaternion.GetChildMemberWithName('w').GetValue(), scale.GetChildMemberWithName('x').GetValue(), scale.GetChildMemberWithName('y').GetValue(), scale.GetChildMemberWithName('z').GetValue())

def MathColor_SummaryProvider(valobj, dict):
    return "{ r = %s, g = %s, b = %s, a = %s }" % (valobj.GetChildMemberWithName('r').GetValue(), valobj.GetChildMemberWithName('g').GetValue(), valobj.GetChildMemberWithName('b').GetValue(), valobj.GetChildMemberWithName('a').GetValue())

def MathRectangle_SummaryProvider(valobj, dict):
    position = valobj.GetChildMemberWithName('m_position')
    size = valobj.GetChildMemberWithName('m_size')
    return "{ x = %s, y = %s, w = %s, h = %s }" % (position.GetChildMemberWithName('x').GetValue(), position.GetChildMemberWithName('y').GetValue(), size.GetChildMemberWithName('x').GetValue(), size.GetChildMemberWithName('y').GetValue())

def MathAngle_SummaryProvider(valobj, dict):
    return "%s" % (valobj.GetChildMemberWithName('m_value').GetValue())

def MathClampedValue_SummaryProvider(valobj, dict):
    return "%s" % (valobj.GetChildMemberWithName('m_value').GetValue())

def Vector_SummaryProvider(valobj, dict):
    provider = SceneriVectorProvider(valobj, dict)
    return "{ size = %d, capacity = %d }" % (provider.num_children(), provider.get_capacity())

def ArrayView_SummaryProvider(valobj, dict):
    provider = SceneriArrayViewProvider(valobj, dict)
    return "{ size = %d }" % (provider.num_children())

def Array_SummaryProvider(valobj, dict):
    provider = SceneriArrayProvider(valobj, dict)
    return "{ size = %d }" % (provider.num_children())

def Guid_SummaryProvider(valobj,internal_dict):
    SBError = lldb.SBError()
    
    # Access the single uint128 member 'm_data' and retrieve as two 64-bit values
    m_data_val = valobj.GetChildMemberWithName('m_data')
    m_data_data = m_data_val.GetData()
    low = m_data_data.GetUnsignedInt64(SBError, 0)
    high = m_data_data.GetUnsignedInt64(SBError, 8)

    # Extract parts directly from high and low
    data1 = (high >> 32) & 0xFFFFFFFF
    data2 = (high >> 16) & 0xFFFF
    data3 = high & 0xFFFF
    data4 = [
        (low >> 56) & 0xFF,
        (low >> 48) & 0xFF,
        (low >> 40) & 0xFF,
        (low >> 32) & 0xFF,
        (low >> 24) & 0xFF,
        (low >> 16) & 0xFF,
        (low >> 8) & 0xFF,
        low & 0xFF
    ]

    # Format the GUID string
    guid_str = "{:08x}-{:04x}-{:04x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}".format(
        data1, data2, data3, 
        data4[0], data4[1], 
        data4[2], data4[3], data4[4], data4[5], data4[6], data4[7]
    )

    return guid_str.upper()

def utf32string_to_string(valobj, error, length):
    length = int(length)
    if length <= 0:
        return ""

    pointer = valobj.GetValueAsUnsigned()
    contents = valobj.GetProcess().ReadMemory(pointer, length * 4, lldb.SBError())

    # lldb does not (currently) support returning unicode from python summary providers,
    # so potentially convert this to ascii by escaping
    string = contents.decode('utf32')
    try:
        return str(string)
    except:
        return string.encode('unicode_escape')

def utf16string_to_string(valobj, error, length):
    length = int(length)
    if length <= 0:
        return ""

    pointer = valobj.GetValueAsUnsigned()
    contents = valobj.GetProcess().ReadMemory(pointer, length * 2, lldb.SBError())

    # lldb does not (currently) support returning unicode from python summary providers,
    # so potentially convert this to ascii by escaping
    string = contents.decode('utf16')
    try:
        return str(string)
    except:
        return string.encode('unicode_escape')

def utf8string_to_string(valobj, error, length):
    length = int(length)
    if length <= 0:
        return ""

    pointer = valobj.GetValueAsUnsigned()
    contents = valobj.GetProcess().ReadMemory(pointer, length, lldb.SBError())

    # lldb does not (currently) support returning unicode from python summary providers,
    # so potentially convert this to ascii by escaping
    string = contents.decode('utf8')
    try:
        return str(string)
    except:
        return string.encode('unicode_escape')

class SceneriStringViewProvider:
    def __init__(self, valobj, dict):
        self.valobj = valobj

    def get_length(self):
        return self.valobj.GetChildMemberWithName('m_size').GetValueAsUnsigned(0)

    def get_begin(self):
        return self.valobj.GetChildMemberWithName('m_pBegin')

    def to_string(self):
        error = lldb.SBError()

        if not self.get_begin() or not self.get_length():
            return u""

        chartype = self.valobj.GetType().GetTemplateArgumentType(0)
        if str(chartype.GetName()) == "char32_t":
            return utf32string_to_string(self.get_begin(), error, self.get_length())
        elif str(chartype.GetName()) == "char16_t":
            return utf16string_to_string(self.get_begin(), error, self.get_length())
        else:
            return utf8string_to_string(self.get_begin(), error, self.get_length())

class SceneriStringProvider:
    def __init__(self, valobj, dict):
        self.valobj = valobj

    def get_length(self):
        return self.valobj.GetChildMemberWithName('m_size').GetValueAsUnsigned(0) - 1

    def get_begin(self):
        allocator = self.valobj.GetChildMemberWithName("m_allocator")
        if str(allocator.GetType().GetName()).startswith("ngine::Memory::DynamicAllocator"):
            return allocator.GetChildMemberWithName('m_pData')
        elif str(allocator.GetType().GetName()).startswith("ngine::Memory::DynamicInlineStorageAllocator"):
            if self.get_length() > 16: # TODO: Figure out way of getting this dynamically
                 return allocator.GetChildMemberWithName('m_pData')
            else:
                 return allocator.GetChildMemberWithName('m_fixedAllocator').GetChildMemberWithName('m_elementStorage').AddressOf()
        else: # assume fixed allocator
            return allocator.GetChildMemberWithName('m_elementStorage').AddressOf()

    def to_string(self):
        error = lldb.SBError()

        if not self.get_begin() or not self.get_length():
            return u""

        chartype = self.valobj.GetType().GetTemplateArgumentType(0)
        if str(chartype.GetName()) == "char32_t":
            return utf32string_to_string(self.get_begin(), error, self.get_length())
        elif str(chartype.GetName()) == "char16_t":
            return utf16string_to_string(self.get_begin(), error, self.get_length())
        else:
            return utf8string_to_string(self.get_begin(), error, self.get_length())

class SceneriVectorProvider:
    def __init__(self, valobj, internal_dict):
        self.valobj = valobj
        self.update()

    def num_children(self):
        return self.size

    def get_buffer(self):
        allocator = self.valobj.GetChildMemberWithName("m_allocator")
        if str(allocator.GetType().GetName()).startswith("ngine::Memory::DynamicAllocator"):
            return allocator.GetChildMemberWithName('m_pData')
        elif str(allocator.GetType().GetName()).startswith("ngine::Memory::DynamicInlineStorageAllocator"):
            if self.get_capacity() > 16: # TODO: Figure out way of getting this dynamically
                 return allocator.GetChildMemberWithName('m_pData')
            else:
                 return allocator.GetChildMemberWithName('m_fixedAllocator').GetChildMemberWithName('m_elementStorage').AddressOf()
        else: # assume fixed allocator
            return allocator.GetChildMemberWithName('m_elementStorage').AddressOf()

    def get_capacity_member(self):
        allocator = self.valobj.GetChildMemberWithName("m_allocator")
        if str(allocator.GetType().GetName()).startswith("ngine::Memory::DynamicAllocator"):
            return allocator.GetChildMemberWithName("m_capacity")
        elif str(allocator.GetType().GetName()).startswith("ngine::Memory::DynamicInlineStorageAllocator"):
            return allocator.GetChildMemberWithName("m_capacity")
        else: # assume fixed allocator
            return allocator.GetChildMemberWithName("Capacity")

    def get_capacity(self):
       return self.get_capacity_member().GetValueAsUnsigned(0)

    def get_child_index(self, name):
        return int(name.lstrip('[').rstrip(']'))

    def get_child_at_index(self, index):
        offset = index * self.data_size
        child = self.buffer.CreateChildAtOffset('[' + str(index) + ']', offset, self.data_type)
        return child

    def update(self):
        self.buffer = self.get_buffer()
        self.size = self.valobj.GetChildMemberWithName('m_size').GetValueAsUnsigned(0)
        self.capacity = self.get_capacity_member().GetValueAsUnsigned(0)
        self.data_type = self.valobj.GetType().GetTemplateArgumentType(0)
        self.data_size = self.data_type.GetByteSize()

    def has_children(self):
        return True;


class SceneriArrayViewProvider:
    def __init__(self, valobj, internal_dict):
        self.valobj = valobj
        self.update()

    def num_children(self):
        return self.size

    def get_buffer(self):
        return self.valobj.GetChildMemberWithName("m_pBegin")

    def get_capacity(self):
       return self.size

    def get_child_index(self, name):
        return int(name.lstrip('[').rstrip(']'))

    def get_child_at_index(self, index):
        offset = index * self.data_size
        child = self.buffer.CreateChildAtOffset('[' + str(index) + ']', offset, self.data_type)
        return child

    def update(self):
        self.buffer = self.get_buffer()
        self.size = self.valobj.GetChildMemberWithName('m_size').GetValueAsUnsigned(0)
        self.data_type = self.valobj.GetType().GetTemplateArgumentType(0)
        self.data_size = self.data_type.GetByteSize()

    def has_children(self):
        return True;

class SceneriArrayProvider:
    def __init__(self, valobj, internal_dict):
        self.valobj = valobj
        self.update()

    def num_children(self):
        return self.size

    def get_buffer(self):
        return self.valobj.GetChildMemberWithName("m_data").AddressOf()

    def get_capacity(self):
       return self.size

    def get_child_index(self, name):
        return int(name.lstrip('[').rstrip(']'))

    def get_child_at_index(self, index):
        offset = index * self.data_size
        child = self.buffer.CreateChildAtOffset('[' + str(index) + ']', offset, self.data_type)
        return child

    def update(self):
        self.buffer = self.get_buffer()
        self.size = self.valobj.GetChildMemberWithName("Size").GetValueAsUnsigned(0)
        self.data_type = self.valobj.GetType().GetTemplateArgumentType(0)
        self.data_size = self.data_type.GetByteSize()

    def has_children(self):
        return True;
