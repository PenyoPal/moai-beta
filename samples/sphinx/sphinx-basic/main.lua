MOAISim.openWindow ( "test", 320, 480 )

print ( "MOAISphinx", MOAISphinx )

MOAISphinx:inititalize_sphinx ( )

viewport = MOAIViewport.new ()
viewport:setSize ( 320, 480 )
viewport:setScale ( 320, -480 )

layer = MOAILayer2D.new ()
layer:setViewport ( viewport )
MOAISim.pushRenderPass ( layer )

gfxQuad = MOAIGfxQuad2D.new ()
gfxQuad:setTexture ( "moai.png" )
gfxQuad:setRect ( -128, -128, 128, 128 )
gfxQuad:setUVRect ( 0, 0, 1, 1 )

prop = MOAIProp2D.new ()
prop:setDeck ( gfxQuad )
layer:insertProp ( prop )

prop:moveRot ( 360, 1.5 )

print ( "Creating buffer" )
local sb = MOAIUntzSampleBuffer.new ( )
local nChan = 1
local freq = 441000
local totalLenSec = 5
sb:prepareBuffer ( nChan, freq * totalLenSec, freq )

local sampler = MOAIAudioSampler.new ( )
print ( "Methods on sampler" )
sampler:setFrequency ( freq )
sampler:setNumChannels ( nChan )
sampler:start ( )
print ( "Recording" )
local data = {}
local numSamples = freq * totalLenSec
for i=0,numSamples do
  data[i] = sampler:read ( )
end
print ( "Done" )
print ( #data )
sb:setData ( data, 1 )
print ( "Playing sound" )
local s = MOAIUntzSound.new ( )
s:load ( samplebuf )
s:setVolume ( 1 )
s:setLooping ( true )
s:play ( )


