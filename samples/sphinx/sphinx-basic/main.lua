-- Consts [[
WORLD_W =  320
WORLD_H = 480

--MOAIDebugLines.setStyle ( MOAIDebugLines.TEXT_BOX, 2, 0.5, 0.5, 0.5 )

MOAISim.openWindow ( "test", WORLD_H, WORLD_W )

DEVICE_HEIGHT = MOAIEnvironment.screenHeight
DEVICE_WIDTH = MOAIEnvironment.screenWidth

SCREEN_X_OFFSET = 0
SCREEN_Y_OFFSET = 0

local gameAspect = WORLD_H / WORLD_W
local realAspect = DEVICE_HEIGHT / DEVICE_WIDTH

if realAspect > gameAspect then
  SCREEN_WIDTH = DEVICE_WIDTH
  SCREEN_HEIGHT = DEVICE_WIDTH * gameAspect
else
  SCREEN_WIDTH = DEVICE_HEIGHT / gameAspect
  SCREEN_HEIGHT = DEVICE_HEIGHT
end

if SCREEN_WIDTH < DEVICE_WIDTH then
  SCREEN_X_OFFSET = ( DEVICE_WIDTH - SCREEN_WIDTH ) * 0.5
end

if SCREEN_HEIGHT < DEVICE_HEIGHT then
  SCREEN_Y_OFFSET = ( DEVICE_HEIGHT - SCREEN_HEIGHT ) * 0.5
end

PHRASES = {
 "到 上面 看一 看",
 "到 下面",
 "我 看到 你的 你的 记不住 是 上面 或者 下面",
 "往 左 走",
 "往 右",
 "我 应该 往 左 或者 往 右",
 "你 应该 进 入口",
 "出口",
 "哎呀  我 应该 进 那一个 门",
 "这个 星星 更 可爱",
 "那个 星星",
 "你 觉得 那一个 星星 更 可爱",
 "可以",
 "我 很 难过 我 想 找 我的 你的 你 能 帮 我",
 "哎呦 不 好 不好 我的 苹果",
 "苹果 在 哪一棵 树 后面",
 "大 树 后面",
 "中等 大小 的 树",
 "小 树",
 "苹果 在 里面 或者 外面",
 "苹果 在 里面",
 "在 外面",
 "他们 为什么 不 高兴",
 "他们 太 热 了",
 "太 冷 了",
 "救命 救命",
 "谢谢你 我 可以 吃饭 了"
}

CHAR_TO_PINYIN = {
   ["呦"] = "you",
   ["我"] = "wo",
   ["应"] = "ying",
   ["该"] = "gai",
   ["进"] = "jin",
   ["那"] = "na",
   ["一"] = "yi",
   ["个"] = "ge",
   ["门"] = "men",
   ["你"] = "ni",
   ["入"] = "ru",
   ["口"] = "kou",
   ["出"] = "chu",
   ["很"] = "hen",
   ["难"] = "nan",
   ["过"] = "guo",
   ["想"] = "xiang",
   ["找"] = "zhao",
   ["的"] = "de",
   ["苹"] = "ping",
   ["果"] = "guo",
   ["能"] = "neng",
   ["帮"] = "bang",
   ["吗"] = "ma",
   ["可"] = "ke",
   ["以"] = "yi",
   ["看"] = "kan",
   ["到"] = "dao",
   ["记"] = "ji",
   ["不"] = "bu",
   ["住"] = "zhu",
   ["是"] = "shib",
   ["上"] = "shang",
   ["面"] = "mian",
   ["或"] = "huo",
   ["者"] = "zhe",
   ["下"] = "xia",
   ["哎"] = "asai",
   ["呀"] = "ya",
   ["觉"] = "juxe",
   ["得"] = "de",
   ["星"] = "xing",
   ["更"] = "geng",
   ["爱"] = "asai",
   ["这"] = "zhe",
   ["往"] = "wang",
   ["左"] = "zuo",
   ["右"] = "you",
   ["走"] = "zou",
   ["好"] = "hao",
   ["在"] = "zai",
   ["太"] = "tai",
   ["热"] = "re",
   ["了"] = "le",
   ["冷"] = "leng",
   ["树"] = "shu",
   ["小"] = "xaio",
   ["大"] = "da",
   ["后"] = "hou",
   ["中"] = "zhong",
   ["等"] = "deng",
   ["里"] = "li",
   ["外"] = "wai",
   ["他"] = "ta",
   ["们"] = "men",
   ["高"] = "gao",
   ["兴"] = "xing",
   ["救"] = "jiu",
   ["命"] = "ming",
   ["吃"] = "chib",
   ["饭"] = "fan",
   ["哪"] = "na",
   ["棵"] = "ke",
   ["谢"] = "xie",
   ["为"] = "wei"
}

local function phrase2pinyin ( chars )
  local pinyin = ""
  for uchar in string.gfind( chars, "([%z\1-\127\194-\244][\128-\191]*)" ) do
    pinyin = pinyin .. " " .. ( CHAR_TO_PINYIN[uchar] or " " )
  end
  return pinyin
end

math.random ( )
local function chooseWord ( )
  return PHRASES[math.random ( #PHRASES )]
end

viewport = MOAIViewport.new ()
viewport:setSize ( SCREEN_X_OFFSET, SCREEN_Y_OFFSET, SCREEN_X_OFFSET + SCREEN_WIDTH, SCREEN_Y_OFFSET + SCREEN_HEIGHT )
viewport:setScale ( WORLD_W, WORLD_H )

if SCREEN_WIDTH <= 480 then
  assetSize = 480
elseif SCREEN_WIDTH <= 960 then
  assetSize = 960
elseif SCREEN_WIDTH <= 1024 then
  assetSize = 1024
else
  assetSize = 2048
end

-- ]]

-- Textboxes [[

enFont = MOAIFont.new ()
enFont:load ( 'arialbd.ttf' )
enFontCache = MOAIGlyphCache.new ( )
enFont:setCache ( enFontCache )

zhFont = MOAIFont.new ( )
zhFont:load ( 'Kai.ttf' )
zhFontCache = MOAIGlyphCache.new ( )
zhFont:setCache ( zhFontCache )

print ( "MOAISphinx", MOAISphinx )
--MOAISphinx: initialize_sphinx ( )

layer = MOAILayer2D.new ()
layer:setViewport ( viewport )
MOAISim.pushRenderPass ( layer )

enTextBox = MOAITextBox.new ( )
enTextBox:setColor ( 255/255, 250/255, 199/255 )
enTextBox:setFont ( zhFont )
enTextBox:setTextSize ( 15 )
enTextBox:setAlignment ( MOAITextBox.CENTER_JUSTIFY )
enTextBox:setYFlip ( true )
enTextBox:setRect ( -WORLD_W / 2, 0, WORLD_W / 2, WORLD_H / 2 )
enTextBox:setString ( "Tap to record" )
layer:insertProp ( enTextBox )

zhTextBox = MOAITextBox.new ( )
zhTextBox:setColor ( 255/255, 250/255, 199/255 )
zhTextBox:setFont ( zhFont )
zhTextBox:setTextSize ( 15 )
zhTextBox:setAlignment ( MOAITextBox.CENTER_JUSTIFY )
zhTextBox:setYFlip ( true )
zhTextBox:setRect ( -WORLD_W / 2, WORLD_H / 2 - 30, WORLD_W / 2, 0 )
zhTextBox:setString ( "" )
layer:insertProp ( zhTextBox )
-- ]]


local nChan = 1
local unit = 3
local freq = 16000

MOAIUntzSystem.initialize ( freq, 8192, MOAIUntzSystem.RECORDABLE )

-- Helper functions [[
local function setPrompt ( )
  local promptStr = "Say one of: "
  choices = { }
  for i=1,3 do
    local choice = chooseWord ( )
    table.insert ( choices, choice )
    promptStr = promptStr .. "\nor\n" .. choice .. "(" .. phrase2pinyin ( choice ) .. ")"
  end
  promptStr = promptStr .. "\n(tap to record, tap again to analyze)"
  enTextBox:setString ( promptStr )
end

local function reduce ( tbl, init, func )
  local acc = init
  for i, v in pairs ( tbl ) do
    acc = func ( acc, v )
  end
  return acc
end

-- ]]]

-- Edit distance functions [[[

local function edit_distance ( word1, word2 )
  dist = 0
  for i = 1, math.max( #word1, #word2 ) do
    if i > #word1 then
      return dist + #word2 - i
    elseif i > #word2 then
      return dist + #word1 - i
    end
    if word1:sub(i, i) ~= word2:sub(i, i) then
      dist = dist + 1
    end
  end
  return dist
end

local function determine_phrase ( hyps )
  local best = nil
  local best_dist = 3000
  for i, choice in ipairs ( choices ) do
    local pchoice = phrase2pinyin ( choice )
    for j, hyp in ipairs ( hyps ) do
      local phyp = phrase2pinyin ( hyp )
      local dist = edit_distance ( phyp, pchoice )
      if dist < best_dist then
        best_dist = dist
        best = choice
      end
    end
  end
  return best
end

-- ]]

-- Recording functions [[[
local Recorder = {
  recording = false, tape = {}, nChan = nChan, freq = freq, unit = unit,
  sampleMax = 0
}

function Recorder:addData ( )
  local data = self.sampler:read ( "short" )
  if data and type ( data ) == "table" then
    print ( "Recording..." )
    table.insert ( self.tape,  data )
    for i, val in ipairs ( data ) do
      if math.abs ( val ) > self.sampleMax then
        self.sampleMax = math.abs ( val )
      end
    end
  end
end

function Recorder:init ( )
  self.sampler = MOAIAudioSampler.new ( )
  self.sampler:setFrequency ( self.freq )
  self.sampler:setNumChannels ( self.nChan )
  self.sampler:prepareBuffer ( unit, 10 )
end

function Recorder:recordSample ( )
  self.recording = true
  self.tape = { }
  self.sampler:start ( )

  self.th = MOAIThread.new ( )
  self.th:run ( function ( )
    while self.recording do
      self:addData ( )
      coroutine.yield ( )
    end
  end )
end

function Recorder:getSample ( )
  local threshold = self.sampleMax * 0.01
  local skippedEmpty = false
  local sample = {}
  for i = 1, #self.tape do
    for j = 1, #self.tape[i] do
      if skippedEmpty then
        table.insert ( sample, self.tape[i][j] )
      elseif math.abs ( self.tape[i][j] ) > threshold then
        skippedEmpty = true
      end
    end
  end
  print ( "Giving sample of size ", #sample )
  return sample
end

function Recorder:stopSampling ( )
  self.recording = false
  self.sampler:flush ( )
  self:addData ( )
  self.sampler:stop ( )
  print ( "Done sampling ", #self.tape, "samples" )
end
-- ]]]

-- Playback [[
local function playback ( )
  zhTextBox:setString ( "Playing back..." )
  local sample = Recorder:getSample ( )
  print ( "Sample is length", #sample )
  local audio_sample = {}
  for i, v in ipairs ( sample ) do
    audio_sample[i] = v / 32767
  end
  print ( "Audio sample size", #audio_sample )

  local playbuf = MOAIUntzSampleBuffer.new ( )
  playbuf:prepareBuffer ( nChan, freq * unit * 3, freq )

  local snd = MOAIUntzSound.new ( )
  snd:load ( playbuf )
  snd:setVolume ( 1 )
  snd:setLooping ( false )
  snd:play ( )

  playbuf:setData ( audio_sample, 1 )
  snd:setPosition ( 0 )
  snd:play ( )

  zhTextBox:setString ( "Analyzing..." )

  local hyp1, hyp2, hyp3 = MOAISphinx:analyze_utterance ( #sample, sample )
  local best_guess = determine_phrase ( { hyp1, hyp2, hyp3 } )
  local msg = "Sounds like (tap to record again)\n"
  msg = msg .. hyp1 .. phrase2pinyin ( hyp1 ) .."\nor\n" .. hyp2 .. phrase2pinyin ( hyp2 ) .. "\nor\n" .. hyp3 .. phrase2pinyin ( hyp3 )
  msg = msg .. "\nBest match is\n" .. best_guess .. "(" .. phrase2pinyin ( best_guess ) .. ")"
  zhTextBox:setString ( msg )

end
-- ]]

-- Input callback [[
local recording = false
if MOAIInputMgr.device.touch then
  print ( "Touch device go" )
  -- For a touch interface
  MOAIInputMgr.device.touch:setCallback (
  function ( eventType, idx, x, y, tapCount )
    print ( "Pressed" )
    if eventType == MOAITouchSensor.TOUCH_UP then
      if not recording then
        recording = true
        print ( "Starting recording" )
        zhTextBox:setString ( "Recording..." )
        recording = true
        Recorder:init ( )
        print ( "Inited" )
        Recorder:recordSample ( )
        print ( "Recording..." )
      else
        recording = false
        print ( "Stopping recording" )
        Recorder:stopSampling ( )
        playback ( )
      end
    end
  end )
end

-- ]]

setPrompt ( )

zhTextBox:setString ( "Waiting...")
