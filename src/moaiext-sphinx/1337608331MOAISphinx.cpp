#include <moaiext-sphinx/MOAISphinx.h>
#include <pocketsphinx.h>

int MOAISphinx::_analyze_utterance(luaState* L) {

};

MOAISphinx::

void MOAISphinx::RegisterLuaClass(MOAILuaState& state) {
  MOAINode::RegisterLuaClass(state);
}

void MOAISphinx::RegisterluaFuncs(MOAILuaState& state) {
  MOAINode::RegisterLuaFuncs(state);
  luaL_Reg regTable[] = {
    {"analyze_sample", _analyze_sample},
    {NULL, NULL}
  };

  luaL_register(state, 0, regTable);
}
