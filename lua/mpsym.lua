local function is_number(obj)
  return type(obj) == 'number'
end

local function is_integer(obj)
  return is_number(obj) and math.floor(obj) == obj
end

local function to_integer(obj)
  obj = tonumber(obj)

  if not is_integer(obj) then
    error("not convertible to integer: '" .. obj .. "'")
  end

  return obj
end

local function is_float(obj)
  return is_number(obj) and not is_integer(obj)
end

local function to_float(obj)
  obj = tonumber(obj)

  if not is_float(obj) then
    error("not convertible to float: '" .. obj .. "'")
  end

  return obj
end

local function is_boolean(obj)
  return type(obj) == 'boolean'
end

local function is_string(obj)
  return type(obj) == 'string'
end

local function is_table(obj)
  return type(obj) == 'table'
end

local function is_array(obj, min_len, max_len)
  local keys = {}
  local max_key = 0

  for k, _ in pairs(obj) do
    if not is_number(k) then
      return false
    end

    keys[k] = true

    if k > max_key then
      max_key = k
    end
  end

  for i = 1, max_key do
    if keys[i] == nil then
      return false
    end
  end

  if min_len ~= nil and max_key < min_len then
    return false
  end

  if max_len ~= nil and max_key > max_len then
    return false
  end

  return true
end

local function dump(obj)
  if not is_table(obj) then
    if is_string(obj) then
      return "'" .. obj .. "'"
    end

    return tostring(obj)
  end

  local keys = {}
  for k, _ in pairs(obj) do
    table.insert(keys, k)
  end

  table.sort(keys, function(a, b)
    if is_number(a) and is_number(b) then
      return a < b
    else
      if is_number(a) then
        return true
      elseif is_number(b) then
        return false
      end

      return tostring(a) < tostring(b)
    end
  end)

  local str = '{'

  for _, k in ipairs(keys) do
    if not is_number(k) then
      str = str .. dump(k) .. ": "
    end

    str = str .. dump(obj[k]) .. ', '
  end

  str = string.sub(str, 1, #str - 2) .. '}'

  return str
end

local function is_processor(obj)
  if not is_array(obj, 2, 2) then
    return false
  end

  local has_id = is_number(obj[1])
  local has_type = is_string(obj[2])

  return has_id and has_type
end

local function is_channel(obj)
  if not is_array(obj, 2, 3) then
    return false
  end

  local has_source = is_number(obj[1])
  local has_target = is_number(obj[2])
  local has_type = is_string(obj[3])

  return has_source and has_target and has_type
end

local function is_arch_graph(obj)
  if not is_table(obj) then
    return false, "not a table"
  end

  if obj.processors == nil and obj.clusters == nil then
    return false, "missing 'processors/clusters' element"
  end

  if obj.clusters ~= nil then
    obj.processors = obj.clusters
    obj.clusters = nil
  end

  if obj.channels == nil then
    return false, "missing 'channels' element"
  end

  for k, v in pairs(obj) do
    -- check that directed element is well formed
    if k == 'directed' then
      if not is_boolean(v) then
        return false, "'directed' element is not a boolean"
      end
    -- check that processing elements are well formed
    elseif k == 'processors' then
      if not is_array(v) then
        return false, "'processors' element is not an array"
      end

      for _, p in ipairs(v) do
        if not is_processor(p) then
          return false, "malformed processor: " .. dump(p)
        end
      end
    -- check that channels are well formed
    elseif k == 'channels' then
      if not is_array(v) then
        return false, "'channels' element is note an array"
      end

      for _, c in ipairs(v) do
        if not is_channel(c) then
          return false, "malformed channel: " .. dump(c)
        end
      end
    elseif string.sub(k, 1, 1) ~= '_' then
      return false, "unexpected element: " .. dump(k)
    end
  end

  -- check that processing elements are unique
  local processor_ids = {}

  for _, p in ipairs(obj.processors) do
    if processor_ids[p[1]] == true then
      return false, "processor id is not unique: " .. dump(p)
    end

    processor_ids[p[1]] = true
  end

  -- check that channels refer to existing processing elements
  for _, c in ipairs(obj.channels) do
    if processor_ids[c[1]] == nil or processor_ids[c[2]] == nil then
      return false, "channel refers to non existing processor(s): " .. dump(c)
    end
  end

  return true
end

function is_arch_graph_cluster(obj)
  if not is_table(obj) or not is_array(obj, 2) then
    return false, "not a table"
  end

  local i = 1
  for _, v in ipairs(obj) do
    --if not is_arch_graph_system(v) then
    if not is_arch_graph(v) then
      return false, "element nr. " .. i .. " is not a valid ArchGraphSystem"
    end
    i = i + 1
  end

  return true
end

function is_arch_uniform_super_graph(obj)
  if not is_table(obj) then
    return false, "not a table"
  end

  if obj.super_graph == nil then
    return false, "missing 'super_graph' element"
  end

  if obj.proto == nil then
    return false, "missing 'proto' element"
  end

  for k, v in pairs(obj) do
    if k == 'super_graph' or k == 'proto' then
      if not is_arch_graph_system(v) then
        return false, "'" .. k .. "' element is not a valid ArchGraphSystem"
      end
    else
      return false, "unexpected element: " .. dump(k)
    end
  end

  return true
end

function is_arch_graph_system(obj)
  return is_arch_graph(obj) or
         is_arch_graph_cluster(obj) or
         is_arch_uniform_super_graph(obj)
end

local ArchGraph = { metaname = 'ArchGraph' }
ArchGraph.__index = ArchGraph

local ArchGraphCluster = { metaname = 'ArchGraphCluster' }
ArchGraph.__index = ArchGraph

local ArchUniformSuperGraph = { metaname = 'ArchUniformSuperGraph' }
ArchGraph.__index = ArchGraph

function ArchGraph:create(obj)
  local valid, err = is_arch_graph(obj)

  if not valid then
    error("malformed ArchGraph: " .. err)
  end

  setmetatable(obj, self)
  obj.__index = self

  obj._directed = (obj.directed == nil) and true or obj.directed

  obj._num_processors = #obj.processors
  obj._num_channels = #obj.channels

  obj._processor_types = {}

  local processor_type_set = {}
  for _, p in ipairs(obj.processors) do
    local ptype = p[2]

    if not processor_type_set[ptype] then
      processor_type_set[ptype] = true
      table.insert(obj._processor_types, ptype)
    end
  end

  obj._channel_types = {}

  local channel_type_set = {}
  for _, c in ipairs(obj.channels) do
    local ctype = c[3]

    if not channel_type_set[ctype] then
      channel_type_set[ctype] = true
      table.insert(obj._channel_types, ctype)
    end
  end

  return obj

end

function ArchGraphCluster:create(obj)
  local valid, err = is_arch_graph_cluster(obj)

  if not valid then
    error("malformed ArchGraphCluster: " .. err)
  end

  setmetatable(obj, self)
  obj.__index = self

  obj._num_processors = 0
  obj._num_channels = 0

  for _, v in ipairs(obj) do
    obj._num_processors = obj._num_processors + v._num_processors
    obj._num_channels = obj._num_channels + v._num_channels
  end

  return obj
end

function ArchUniformSuperGraph:create(obj)
  local valid, err = is_arch_uniform_super_graph(obj)

  if not valid then
    error("malformed ArchUniformSuperGraph" .. err)
  end

  setmetatable(obj, self)
  obj.__index = self

  obj._num_processor = obj.super_graph._num_processors * obj.proto._num_processors

  obj._num_channels = obj.super_graph._num_channels +
                      obj.super_graph._num_processors * obj.proto._num_channels

  return obj
end

function identical_processors(num, ptype)
  local processors = {}
  for p = 1,num do
    table.insert(processors, {p, ptype})
  end

  return processors
end

function combine_processors(...)
  local arg = {...}

  local processors = {}

  for i = 1,#arg do
    local p = arg[i]

    if is_processor(p) then
      table.insert(processors, p)
    else
      if not is_array(p) then
        error("argument " .. i .. " is not an array of processors")
      end

      for i, p_ in ipairs(p) do
        if not is_processor(p_) then
          error("argument " .. i .. " is not an array of processors")
        end

        table.insert(processors, p_)
      end
    end
  end

  return processors
end

function append_processors(old_processors, new_processors)
  local num_old_processors = #old_processors
  local new_processors_shifted = {}

  for _, p in ipairs(new_processors) do
    local p_ = {p[1] + num_old_processors, p[2]}
    table.insert(new_processors_shifted, p_)
    table.insert(old_processors, p_)
  end

  return new_processors_shifted
end

function insert_channel(channels, from, to, chtype, directed)
  if directed == nil then
    directed = true
  end

  table.insert(channels, {from, to, chtype})

  if not directed then
    table.insert(channels, {to, from, chtype})
  end
end

function linear_channels(processors, ctype, directed)
  local channels = {}

  for i = 1, #processors - 1 do
    insert_channel(channels, processors[i][1], processors[i + 1][1], ctype, directed)
  end

  return channels
end

function cyclic_channels(processors, ctype, directed)
  local channels = linear_channels(processors, ctype)

  insert_channel(channels, processors[#processors][1], processors[1][1], ctype, directed)

  return channels
end

function self_connected_channels(processors, ctype, directed)
  local channels = {}

  for i = 1,#processors do
    insert_channel(channels, processors[i][1], processors[i][1], ctype, directed)
  end

  return channels
end

function fully_connected_channels(processors, ctype)
  local channels = {}

  for i = 1,#processors do
    for j = i+1,#processors do
      insert_channel(channels, processors[i][1], processors[j][1], ctype, false)
    end
  end

  return channels
end

function fully_cross_connected_channels(processors1, processors2, ctype)
  local channels = {}

  for i = 1,#processors1 do
    for j = 1,#processors2 do
      insert_channel(channels, processors1[i][1], processors2[j][1], ctype, false)
    end
  end

  return channels
end

function grid_channels(processors, ctype, height, width)
  local channels = {}

  if width == nil and height == nil then
    local dim = math.floor(math.sqrt(#processors))

    if #processors ~= dim * dim then
      error("failed to determine grid height/width")
    end

    width = dim
    height = dim
  else
    if width == nil then
      if #processors % height ~= 0 then
        error("failed to determine grid width")
      end

      width = #processors / height
    elseif height == nil then
      if #processors % height ~= 0 then
        error("failed to determine grid width")
      end

      height = #processors / width
    end
  end

  local channels = {}

  for i = 1,height do
    for j = 1,width do
      local k = (i - 1) * width + j

      -- horizontal channels
      if j < width then
        insert_channel(channels, processors[k][1], processors[k + 1][1], ctype, false)
      end

      -- vertical channels
      if i < height then
        insert_channel(channels, processors[k][1], processors[k + width][1], ctype, false)
      end
    end
  end

  return channels
end

function append_channels(old_channels, new_channels)
  for _, c in ipairs(new_channels) do
    table.insert(old_channels, c)
  end
end

function parse_args(args, types)
  if #args ~= #types then
    error("expected " .. #types .. " command line arguments but got " .. #args)
  end

  local ret = {}

  for i = 1,#types do
    local t = types:sub(i, i)
    if t == 'i' then
      table.insert(ret, to_integer(args[i]))
    elseif t == 'f' then
      table.insert(ret, to_float(args[i]))
    elseif t == 's' then
      table.insert(ret, args[i])
    else
      error("unknown type identifier '" .. t .. "'")
    end
  end

  return ret
end

return {
  ArchGraph = ArchGraph,
  ArchGraphCluster = ArchGraphCluster,
  ArchUniformSuperGraph = ArchUniformSuperGraph,
  identical_processors = identical_processors,
  identical_clusters = identical_processors,
  combine_processors = combine_processors,
  append_processors = append_processors,
  combine_clusters = combine_processors,
  append_clusters = append_processors,
  linear_channels = linear_channels,
  cyclic_channels = cyclic_channels,
  self_connected_channels = self_connected_channels,
  fully_connected_channels = fully_connected_channels,
  fully_cross_connected_channels = fully_cross_connected_channels,
  grid_channels = grid_channels,
  append_channels = append_channels,
  parse_args = parse_args
}
