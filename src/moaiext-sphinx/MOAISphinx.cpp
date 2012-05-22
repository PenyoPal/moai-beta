#include <moaiext-sphinx/MOAISphinx.h>
#include <pocketsphinx.h>

int MOAISphinx::_initialize_sphinx(lua_State* L) {
  MOAI_LUA_SETUP(MOAISphinx, "U")

  cmd_ln_t *config;

  config = cmd_ln_init(NULL, ps_args(), TRUE,
      "-hmm", MODEL_DIR "tdt_sc_8kadapt",
      "-lm", MODEL_DIR "penyo.lm.dmp",
      "-dict", MODEL_DIR "penyo.dic",
      NULL);
  if (config == NULL) {
    perror("Failed to create config");
    return 0;
  }

  self->ps = ps_init(config);
  if (self->ps == NULL) {
    perror("Failed to initialize decoder");
    return 0;
  }
  return 1;
}

int MOAISphinx::_analyze_utterance(lua_State* L) {
  MOAI_LUA_SETUP(MOAISphinx, "US")
  cc8* filename = state.GetValue< cc8* >(2, "");

  FILE *fh;

  fh = fopen(filename, "rb");
  if (fh == NULL) {
    perror("Failed to open input wav");
    return 0;
  }

  int rv = ps_decode_raw(self->ps, fh, NULL, -1);
  if (rv < 0) {
    perror("Error decoding input audio");
    return 0;
  }
  fclose(fh);

  const char *hyps[3] = { NULL, NULL, NULL };
  const char *uttid;
  int32 scores[3] = { 0, 0, 0 };
  hyps[0] = ps_get_hyp(self->ps, scores, &uttid);

  self->nbest = ps_nbest(self->ps, 0, -1, NULL, NULL);
  if (self->nbest == NULL) {
    perror("Error creating hypothesis iterator");
    return 0;
  }

  ps_nbest_next(self->nbest); ps_nbest_next(self->nbest);
  for (int i = 1; i < 3 && self->nbest; i++, ps_nbest_next(self->nbest)) {
    hyps[i] = ps_nbest_hyp(self->nbest, scores + i);
    if (hyps[i] == NULL) break;
  }
  for (int i = 0; i < 3; i++) {
    if (hyps[i]) {
      lua_pushlstring ( state, hyps[i], strlen(hyps[i]) );
    } else {
      lua_pushnil ( state ) ;
    }
  }

  return 1;
}

MOAISphinx::MOAISphinx() {
	RTTI_BEGIN
	RTTI_EXTEND ( MOAILuaObject )
	RTTI_END
}

MOAISphinx::~MOAISphinx() {
}

void MOAISphinx::RegisterLuaClass(MOAILuaState& state) {
  luaL_Reg regTable[] = {
		{"analyze_sample", _analyze_utterance},
		{"inititalize_sphinx", _initialize_sphinx},
		{NULL, NULL}
  };

  luaL_register(state, 0, regTable);
}
