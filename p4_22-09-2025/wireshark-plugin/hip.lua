-- EtherType value (e.g., 0x1234)
local custom_ethertype = 0xBB88
myip_protocol = Proto("MyIP",  "MyIP Protocol")

myip_dest = ProtoField.uint8("myip.destination", "Destination", base.DEC)
myip_src = ProtoField.uint8("myip.source", "Source", base.DEC)
myip_len = ProtoField.uint8("myip.length", "Length", base.DEC)
myip_ver = ProtoField.uint8("myip.version", "Version", base.DEC)
myip_type = ProtoField.uint8("myip.type", "Type", base.DEC)

myip_data = ProtoField.string("myip.data", "Data")

myip_protocol.fields = { myip_dest, myip_src, myip_len, myip_ver, myip_type, myip_data }

function myip_protocol.dissector(buffer, pinfo, tree)
  length = buffer:len()
  if length == 0 then
    return
  end

  pinfo.cols.protocol = myip_protocol.name

  local subtree = tree:add(myip_protocol, buffer(), "MyIP Protocol Data")

  subtree:add_le(myip_dest, buffer(0,1):bitfield(0, 8))
  subtree:add_le(myip_src, buffer(1,1):bitfield(0, 8))
  subtree:add_le(myip_len, buffer(2,1):bitfield(0, 8))
  subtree:add_le(myip_ver, buffer(3,1):bitfield(0, 4))

  -- Extract the packet type number
  local opcode_number = buffer(3,1):bitfield(4, 4)
  local opcode_name = get_opcode_name(opcode_number)
  subtree:add_le(myip_type, buffer(3,1):bitfield(4, 4)):append_text(" (" .. opcode_name .. ")")

  -- Display raw data as a subtree
  local data_subtree = tree:add(myip_protocol, buffer(), "Data")
  data_subtree:add_le(buffer(4, length - 4), "Data: " .. buffer(4, length - 4):string())
end

function get_opcode_name(opcode)
  local opcode_name = "Unknown"

  if opcode == 1 then
    opcode_name = "HIP_TYPE_SYN"
  elseif opcode == 2 then
    opcode_name = "HIP_TYPE_SYNACK"
  elseif opcode == 3 then
    opcode_name = "HIP_TYPE_DATA"
  else
    error("invalid opcode")
  end

  return opcode_name
end

-- Register the dissector for your custom EtherType
local eth_dissector_table = DissectorTable.get("ethertype")
eth_dissector_table:add(custom_ethertype, myip_protocol)
