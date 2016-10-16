#include "precompiled.h"

// native set_member(_index, any:_member, any:...);
cell AMX_NATIVE_CALL set_member(AMX *amx, cell *params)
{
	enum args_e { arg_count, arg_index, arg_member, arg_value, arg_elem };
	member_t *member = memberlist[params[arg_member]];

	if (unlikely(member == nullptr)) {
		MF_LogError(amx, AMX_ERR_NATIVE, "%s: unknown member id %i", __FUNCTION__, params[arg_member]);
		return FALSE;
	}

	edict_t *pEdict = edictByIndexAmx(params[arg_index]);
	if (unlikely(pEdict == nullptr || pEdict->pvPrivateData == nullptr)) {
		MF_LogError(amx, AMX_ERR_NATIVE, "%s: invalid or uninitialized entity", __FUNCTION__);
		return FALSE;
	}

	cell* value = getAmxAddr(amx, params[arg_value]);
	size_t element = (PARAMS_COUNT == 4) ? *getAmxAddr(amx, params[arg_elem]) : 0;

	const auto table = memberlist_t::members_tables_e(params[arg_member] / MAX_REGION_RANGE);
	if (table == memberlist_t::mt_csplayer) {
		CBasePlayer *pPlayer = g_ReGameFuncs->UTIL_PlayerByIndex(indexOfEdict(pEdict));
		if (unlikely(!pPlayer || !pPlayer->CSPlayer())) {
			return FALSE;
		}

		return set_member(pPlayer->CSPlayer(), member, value, element);
	}

	return set_member(pEdict->pvPrivateData, member, value, element);
}

// native any:get_member(_index, any:_member, any:...);
cell AMX_NATIVE_CALL get_member(AMX *amx, cell *params)
{
	enum args_e { arg_count, arg_index, arg_member, arg_3, arg_4, arg_5 };
	member_t *member = memberlist[params[arg_member]];

	if (unlikely(member == nullptr)) {
		MF_LogError(amx, AMX_ERR_NATIVE, "%s: unknown member id %i", __FUNCTION__, params[arg_member]);
		return FALSE;
	}

	edict_t *pEdict = edictByIndexAmx(params[arg_index]);
	if (unlikely(pEdict == nullptr || pEdict->pvPrivateData == nullptr)) {
		MF_LogError(amx, AMX_ERR_NATIVE, "%s: invalid or uninitialized entity", __FUNCTION__);
		return FALSE;
	}

	cell* dest;
	size_t element;
	size_t length;

	switch (PARAMS_COUNT)
	{
	case 5:
		dest = getAmxAddr(amx, params[arg_3]);
		length = *getAmxAddr(amx, params[arg_4]);
		element = *getAmxAddr(amx, params[arg_5]);
		break;
	case 4:
		dest = getAmxAddr(amx, params[arg_3]);
		length = *getAmxAddr(amx, params[arg_4]);
		element = 0;
		break;
	case 3:
	{
		cell* arg3 = getAmxAddr(amx, params[arg_3]);

		if (isTypeReturnable(member->type)) {
			dest = (member->type != MEMBER_FLOAT) ? nullptr : arg3;
			element = *arg3;
		}
		else {
			dest = arg3;
			element = 0;
		}
		length = 0;
		break;
	}
	default:
		dest = nullptr;
		element = 0;
		length = 0;
		break;
	}

	const auto table = memberlist_t::members_tables_e(params[arg_member] / MAX_REGION_RANGE);
	if (table == memberlist_t::mt_csplayer) {
		CBasePlayer *pPlayer = g_ReGameFuncs->UTIL_PlayerByIndex(indexOfEdict(pEdict));
		if (!pPlayer || !pPlayer->CSPlayer()) {
			return FALSE;
		}

		return get_member(pPlayer->CSPlayer(), member, dest, element, length);
	}

	return get_member(pEdict->pvPrivateData, member, dest, element, length);
}

// native set_member_game(any:_member, any:...);
cell AMX_NATIVE_CALL set_member_game(AMX *amx, cell *params)
{
	enum args_e { arg_count, arg_member, arg_value, arg_elem };

	CHECK_GAMERULES();

	member_t *member = memberlist[params[arg_member]];
	if (unlikely(member == nullptr)) {
		MF_LogError(amx, AMX_ERR_NATIVE, "%s: unknown member id %i", __FUNCTION__, params[arg_member]);
		return FALSE;
	}

	cell* value = getAmxAddr(amx, params[arg_value]);
	size_t element = (PARAMS_COUNT == 4) ? *getAmxAddr(amx, params[arg_elem]) : 0;

	return set_member(g_pGameRules, member, value, element);
}

// native get_member_game(any:_member, any:...);
cell AMX_NATIVE_CALL get_member_game(AMX *amx, cell *params)
{
	enum args_e { arg_count, arg_member, arg_3, arg_4 };

	CHECK_GAMERULES();

	member_t *member = memberlist[params[arg_member]];
	if (unlikely(member == nullptr)) {
		MF_LogError(amx, AMX_ERR_NATIVE, "%s: unknown member id %i", __FUNCTION__, params[arg_member]);
		return FALSE;
	}

	cell* dest;
	size_t element;
	size_t length;

	if (PARAMS_COUNT == 4) {
		dest = getAmxAddr(amx, params[arg_3]);
		length = *getAmxAddr(amx, params[arg_4]);
		element = 0;
	}
	else if (PARAMS_COUNT == 3) {
		cell* arg3 = getAmxAddr(amx, params[arg_3]);

		if (isTypeReturnable(member->type)) {
			dest = (member->type != MEMBER_FLOAT) ? nullptr : arg3;
			element = *arg3;
		}
		else {
			dest = arg3;
			element = 0;
		}
		length = 0;
	}
	else {
		dest = nullptr;
		element = 0;
		length = 0;
	}

	void* data;
	// members of m_VoiceGameMgr
	if (params[arg_member] >= m_msgPlayerVoiceMask && params[arg_member] <= m_UpdateInterval) {
		data = &CSGameRules()->m_VoiceGameMgr;
	} else {
		data = g_pGameRules;
	}

	return get_member(data, member, dest, element, length);
}

// native set_entvar(const index, const EntVars:var, any:...);
cell AMX_NATIVE_CALL set_entvar(AMX *amx, cell *params)
{
	enum args_e { arg_count, arg_index, arg_var, arg_value, arg_elem };
	member_t *member = memberlist[params[arg_var]];

	if (unlikely(member == nullptr)) {
		MF_LogError(amx, AMX_ERR_NATIVE, "%s: unknown member id %i", __FUNCTION__, params[arg_var]);
		return FALSE;
	}
	CHECK_ISENTITY(arg_index);

	edict_t *pEdict = edictByIndexAmx(params[arg_index]);
	if (unlikely(pEdict == nullptr || pEdict->pvPrivateData == nullptr)) {
		MF_LogError(amx, AMX_ERR_NATIVE, "%s: invalid or uninitialized entity", __FUNCTION__);
		return FALSE;
	}

	cell* value = getAmxAddr(amx, params[arg_value]);
	size_t element = (PARAMS_COUNT == 4) ? *getAmxAddr(amx, params[arg_elem]) : 0;

	return set_member(&pEdict->v, member, value, element);
}

// native any:get_entvar(const index, const EntVars:var, any:...);
cell AMX_NATIVE_CALL get_entvar(AMX *amx, cell *params)
{
	enum args_e { arg_count, arg_index, arg_var, arg_3, arg_4 };
	member_t *member = memberlist[params[arg_var]];

	if (unlikely(member == nullptr)) {
		MF_LogError(amx, AMX_ERR_NATIVE, "%s: unknown member id %i", __FUNCTION__, params[arg_var]);
		return FALSE;
	}

	CHECK_ISENTITY(arg_index);

	edict_t *pEdict = edictByIndexAmx(params[arg_index]);
	if (unlikely(pEdict == nullptr || &pEdict->v == nullptr)) {
		MF_LogError(amx, AMX_ERR_NATIVE, "%s: invalid or uninitialized entity", __FUNCTION__);
		return FALSE;
	}

	cell* dest;
	size_t element;
	size_t length;

	if (PARAMS_COUNT == 4) {
		dest = getAmxAddr(amx, params[arg_3]);
		length = *getAmxAddr(amx, params[arg_4]);
		element = 0;
	}
	else if (PARAMS_COUNT == 3) {
		cell* arg3 = getAmxAddr(amx, params[arg_3]);

		if (isTypeReturnable(member->type)) {
			dest = (member->type != MEMBER_FLOAT) ? nullptr : arg3;
			element = *arg3;
		}
		else {
			dest = arg3;
			element = 0;
		}

		length = 0;
	}
	else {
		dest = nullptr;
		element = 0;
		length = 0;
	}

	return get_member(&pEdict->v, member, dest, element, length);
}

// native set_pmove(const PlayerMove:pmove, any:...);
cell AMX_NATIVE_CALL set_pmove(AMX *amx, cell *params)
{
	enum args_e { arg_count, arg_var, arg_value, arg_elem };
	member_t *member = memberlist[params[arg_var]];

	if (unlikely(member == nullptr)) {
		MF_LogError(amx, AMX_ERR_NATIVE, "%s: unknown member id %i", __FUNCTION__, params[arg_var]);
		return FALSE;
	}

	cell* value = getAmxAddr(amx, params[arg_value]);
	size_t element = (PARAMS_COUNT == 4) ? *getAmxAddr(amx, params[arg_elem]) : 0;

	return set_member(g_pMove, member, value, element);
}

// native any:get_pmove(const PlayerMove:pmove, any:...);
cell AMX_NATIVE_CALL get_pmove(AMX *amx, cell *params)
{
	enum args_e { arg_count, arg_var, arg_2, arg_3 };
	member_t *member = memberlist[params[arg_var]];

	if (unlikely(member == nullptr)) {
		MF_LogError(amx, AMX_ERR_NATIVE, "%s: unknown member id %i", __FUNCTION__, params[arg_var]);
		return FALSE;
	}

	cell* dest;
	size_t element;
	size_t length;

	if (PARAMS_COUNT == 3) {
		if (member->type == MEMBER_STRING) {
			dest = getAmxAddr(amx, params[arg_2]);
			length = *getAmxAddr(amx, params[arg_3]);
			element = 0;
		} else {
			dest = getAmxAddr(amx, params[arg_2]);
			element = *getAmxAddr(amx, params[arg_3]);
			length = 0;
		}
	}
	else {
		dest = nullptr;
		element = 0;
		length = 0;
	}

	return get_member(g_pMove, member, dest, element, length);
}

// native set_movevar(const MoveVars:var, any:...);
cell AMX_NATIVE_CALL set_movevar(AMX *amx, cell *params)
{
	enum args_e { arg_count, arg_var, arg_value };
	member_t *member = memberlist[params[arg_var]];

	if (unlikely(member == nullptr)) {
		MF_LogError(amx, AMX_ERR_NATIVE, "%s: unknown member id %i", __FUNCTION__, params[arg_var]);
		return FALSE;
	}

	cell* value = getAmxAddr(amx, params[arg_value]);
	return set_member(g_pMove->movevars, member, value, 0);
}

// native any:get_movevar(const MoveVars:var, any:...);
cell AMX_NATIVE_CALL get_movevar(AMX *amx, cell *params)
{
	enum args_e { arg_count, arg_var, arg_2, arg_3 };
	member_t *member = memberlist[params[arg_var]];

	if (unlikely(member == nullptr)) {
		MF_LogError(amx, AMX_ERR_NATIVE, "%s: unknown member id %i", __FUNCTION__, params[arg_var]);
		return FALSE;
	}

	cell* dest;
	size_t element;
	size_t length;

	if (PARAMS_COUNT == 3) {
		dest = getAmxAddr(amx, params[arg_2]);
		length = *getAmxAddr(amx, params[arg_3]);
		element = 0;
	}
	else {
		dest = nullptr;
		element = 0;
		length = 0;
	}

	return get_member(g_pMove->movevars, member, dest, element, length);
}

// native set_ucmd(const cmd, const UserCmd:var, any:...);
cell AMX_NATIVE_CALL set_ucmd(AMX *amx, cell *params)
{
	enum args_e { arg_count, arg_cmd, arg_var, arg_value };
	member_t *member = memberlist[params[arg_var]];

	if (unlikely(member == nullptr)) {
		MF_LogError(amx, AMX_ERR_NATIVE, "%s: unknown member id %i", __FUNCTION__, params[arg_var]);
		return FALSE;
	}

	cell* cmd = (cell *)params[arg_cmd];
	cell* value = getAmxAddr(amx, params[arg_value]);
	return set_member(cmd, member, value, 0);
}

// native any:get_ucmd(const cmd, const UserCmd:var, any:...);
cell AMX_NATIVE_CALL get_ucmd(AMX *amx, cell *params)
{
	enum args_e { arg_count, arg_cmd, arg_var, arg_3, arg_4 };
	member_t *member = memberlist[params[arg_var]];

	if (unlikely(member == nullptr)) {
		MF_LogError(amx, AMX_ERR_NATIVE, "%s: unknown member id %i", __FUNCTION__, params[arg_var]);
		return FALSE;
	}

	cell* dest;
	size_t element;

	if (PARAMS_COUNT == 3) {
		dest = getAmxAddr(amx, params[arg_3]);
		element = *getAmxAddr(amx, params[arg_4]);
	}
	else {
		dest = (member->type != MEMBER_FLOAT) ? nullptr : getAmxAddr(amx, params[arg_3]);
		element = 0;
	}

	cell* cmd = (cell *)params[arg_cmd];
	return get_member(cmd, member, dest, element);
}

// native set_pmtrace(const tr, const PMTrace:var, any:...);
cell AMX_NATIVE_CALL set_pmtrace(AMX *amx, cell *params)
{
	enum args_e { arg_count, arg_tr, arg_var, arg_value };
	member_t *member = memberlist[params[arg_var]];

	if (unlikely(member == nullptr)) {
		MF_LogError(amx, AMX_ERR_NATIVE, "%s: unknown member id %i", __FUNCTION__, params[arg_var]);
		return FALSE;
	}

	cell* tr = (cell *)params[arg_tr];
	cell* value = getAmxAddr(amx, params[arg_value]);
	return set_member(tr, member, value, 0);
}

// native any:get_pmtrace(const tr, const PMTrace:var, any:...);
cell AMX_NATIVE_CALL get_pmtrace(AMX *amx, cell *params)
{
	enum args_e { arg_count, arg_tr, arg_var, arg_3, arg_4 };
	member_t *member = memberlist[params[arg_var]];

	if (unlikely(member == nullptr)) {
		MF_LogError(amx, AMX_ERR_NATIVE, "%s: unknown member id %i", __FUNCTION__, params[arg_var]);
		return FALSE;
	}

	cell* dest;
	size_t element;

	if (PARAMS_COUNT == 3) {
		dest = getAmxAddr(amx, params[arg_3]);
		element = *getAmxAddr(amx, params[arg_4]);
	}
	else {
		dest = nullptr;
		element = 0;
	}

	cell* tr = (cell *)params[arg_tr];
	return get_member(tr, member, dest, element);
}

AMX_NATIVE_INFO EngineVars_Natives[] =
{
	{ "set_entvar", set_entvar },
	{ "get_entvar", get_entvar },

	{ "set_ucmd", set_ucmd },
	{ "get_ucmd", get_ucmd },

	{ nullptr, nullptr }
};

AMX_NATIVE_INFO ReGameVars_Natives[] =
{
	{ "set_member", set_member },
	{ "get_member", get_member },

	{ "set_member_game", set_member_game },
	{ "get_member_game", get_member_game },

	{ "set_pmove", set_pmove },
	{ "get_pmove", get_pmove },

	{ "set_movevar", set_movevar },
	{ "get_movevar", get_movevar },

	{ "set_pmtrace", set_pmtrace },
	{ "get_pmtrace", get_pmtrace },

	{ nullptr, nullptr }
};

void RegisterNatives_Members()
{
	if (!api_cfg.hasReGameDLL())
		fillNatives(ReGameVars_Natives, [](AMX *amx, cell *params) -> cell { MF_LogError(amx, AMX_ERR_NATIVE, "%s: isn't available", "ReGameDll"); return FALSE; });
	
	g_amxxapi.AddNatives(ReGameVars_Natives);
	g_amxxapi.AddNatives(EngineVars_Natives);
}

BOOL set_member(void* pdata, const member_t *member, cell* value, size_t element)
{
	switch (member->type) {
	case MEMBER_CLASSPTR:
		{
			// native set_member(_index, any:_member, _value, _elem);
			CBaseEntity *pEntity = getPrivate<CBaseEntity>(*value);
			set_member<CBaseEntity *>(pdata, member->offset, pEntity, element);
			return TRUE;
		}
	case MEMBER_EHANDLE:
		{
			// native set_member(_index, any:_member, _value, _elem);
			EHANDLE& ehandle = get_member<EHANDLE>(pdata, member->offset, element);
			edict_t *pEdictValue = edictByIndexAmx(*value);
			ehandle.Set(pEdictValue);
			return TRUE;
		}
	case MEMBER_EDICT:
		{
			// native set_member(_index, any:_member, _value, _elem);
			edict_t *pEdictValue = edictByIndexAmx(*value);
			set_member<edict_t *>(pdata, member->offset, pEdictValue, element);
			return TRUE;
		}
	case MEMBER_VECTOR:
		{
			// native set_member(_index, any:_member, Float:_value[3], _elem);
			Vector& pSource = *(Vector *)value;
			set_member<Vector>(pdata, member->offset, pSource, element);
			return TRUE;
		}
	case MEMBER_STRING:
		{
			// native set_member(_index, any:_member, const source[]);
			if (member->max_size > sizeof(char*)) {
				// char []
				char *source = getAmxString(value);
				char *dest = get_member_direct<char>(pdata, member->offset);
				strncpy(dest, source, member->max_size - 1);
				dest[member->max_size - 1] = '\0';

			} else {
				// char *
				char *source = getAmxString(value);
				char *&dest = get_member<char *>(pdata, member->offset);
				g_ReGameFuncs->ChangeString(dest, source);
			}

			return TRUE;
		}
	case MEMBER_QSTRING:
		{
			char *source = getAmxString(value);
			string_t newstr = (source && source[0] != '\0') ? ALLOC_STRING(source) : 0;
			set_member<string_t>(pdata, member->offset, newstr, element);
			return TRUE;
		}
	case MEMBER_FLOAT:
	case MEMBER_INTEGER:
		{
			// native set_member(_index, any:_member, any:_value, _elem);
			set_member<int>(pdata, member->offset, *value, element);
			return TRUE;
		}
	case MEMBER_SHORT:
		{
			// native set_member(_index, any:_member, _value, _elem);
			set_member<short>(pdata, member->offset, *value, element);
			return TRUE;
		}
	case MEMBER_BYTE:
		{
			// native set_member(_index, any:_member, _value, _elem);
			set_member<byte>(pdata, member->offset, *value, element);
			return TRUE;
		}
	case MEMBER_BOOL:
		{
			// native set_member(_index, any:_member, bool:_value, _elem);
			set_member<bool>(pdata, member->offset, *value != 0, element);
			return TRUE;
		}
	case MEMBER_SIGNALS:
		{
			enum { _Signal, _State };

			// native set_member(_index, any:_member, signals[UnifiedSignals]);
			CUnifiedSignals& signal = get_member<CUnifiedSignals>(pdata, member->offset, element);

			int *pSignals = value;
			signal.m_flSignal = pSignals[_Signal];
			signal.m_flState = pSignals[_State];

			return TRUE;
		}
	case MEMBER_DOUBLE:
		{
			// native set_member(_index, any:_member, any:_value, _elem);
			set_member<double>(pdata, member->offset, *(float *)value, element);
			return TRUE;
		}

	case MEMBER_ENTITY:
	case MEMBER_EVARS:
	case MEBMER_REBUYSTRUCT:
	case MEMBER_PMTRACE:
	case MEBMER_USERCMD:
		return FALSE;

	default: break;
	}

	return FALSE;
}

cell get_member(void* pdata, const member_t *member, cell* dest, size_t element, size_t length)
{
	switch (member->type)
	{
	case MEMBER_CLASSPTR:
		{
			// native any:get_member(_index, any:_member, element);
			auto& pEntity = get_member<CBaseEntity *>(pdata, member->offset, element);
			return pEntity ? indexOfEdict(pEntity->pev) : -1;
		}
	case MEMBER_EHANDLE:
		{
			// native any:get_member(_index, any:_member, element);
			EHANDLE ehandle = get_member<EHANDLE>(pdata, member->offset, element);
			edict_t *pEntity = ehandle.Get();
			return pEntity ? indexOfEdict(pEntity) : -1;
		}
	case MEMBER_EDICT:
		{
			// native any:get_member(_index, any:_member, element);
			edict_t *pEntity = get_member<edict_t *>(pdata, member->offset, element);
			return pEntity ? indexOfEdict(pEntity) : -1;
		}
	case MEMBER_VECTOR:
		{
			// native any:get_member(_index, any:_member, any:output[], element);
			if (!dest)
				return 0;

			*(Vector *)dest = get_member<Vector>(pdata, member->offset, element);
			return 1;
		}
	case MEMBER_STRING:
		{
			// native any:get_member(_index, any:_member, any:output[], maxlength);
			if (unlikely(!dest))
				return 0;

			if (member->max_size > sizeof(char*)) {
				// char []
				const char *src = get_member_direct<char>(pdata, member->offset);
				setAmxString(dest, src, length);
			} else {
				// char *
				const char *src = get_member<const char *>(pdata, member->offset);
				if (src == nullptr) {
					setAmxString(dest, "", 1);
					return 0;
				}
				setAmxString(dest, src, length);
			}

			return 1;
		}
	case MEMBER_QSTRING:
		{
			if (unlikely(!dest))
				return 0;

			string_t str = get_member<string_t>(pdata, member->offset, element);
			if (FStringNull(str)) {
				setAmxString(dest, "", 1);
				return 0;
			}

			setAmxString(dest, STRING(str), length);
			return 1;
		}
	case MEMBER_FLOAT:
	case MEMBER_INTEGER:
		{
			auto& ret = get_member<int>(pdata, member->offset, element);
			if (dest)
				*dest = ret;

			return ret;
		}
	case MEMBER_SHORT:
		// native any:get_member(_index, any:_member, element);
		return get_member<short>(pdata, member->offset, element);
	case MEMBER_BYTE:
		// native any:get_member(_index, any:_member, element);
		return get_member<byte>(pdata, member->offset, element);
	case MEMBER_BOOL:
		// native any:get_member(_index, any:_member, element);
		return get_member<bool>(pdata, member->offset, element);
	case MEMBER_DOUBLE:
		// native any:get_member(_index, any:_member, element);
		return get_member<double>(pdata, member->offset, element);
	case MEMBER_SIGNALS:
		{
			enum { _Signal, _State };

			// native any:get_member(_index, any:_member, signals[2]);
			if (unlikely(!dest))
				return 0;

			CUnifiedSignals& signal = get_member<CUnifiedSignals>(pdata, member->offset);

			int *pSignals = dest;
			pSignals[_Signal] = signal.GetSignal();
			pSignals[_State] = signal.GetState();
			return 1;
		}

	case MEMBER_ENTITY:
	case MEMBER_EVARS:
	case MEBMER_REBUYSTRUCT:
		return 0;

	case MEMBER_PMTRACE:
		// native any:get_member(_index, any:_member, element);
		return (cell)get_member_direct<pmtrace_s>(pdata, member->offset, element);
	case MEBMER_USERCMD:
		// native any:get_member(_index, any:_member, element);
		return (cell)get_member_direct<usercmd_s>(pdata, member->offset, element);

	default: break;
	}

	return 0;
}

bool isTypeReturnable(MType type)
{
	switch (type) {
	case MEMBER_FLOAT:
	case MEMBER_DOUBLE:
	case MEMBER_ENTITY:
	case MEMBER_CLASSPTR:
	case MEMBER_EDICT:
	case MEMBER_INTEGER:
	case MEMBER_SHORT:
	case MEMBER_BYTE:
	case MEMBER_BOOL:
	case MEMBER_EVARS:
	case MEMBER_PMTRACE:
	case MEBMER_USERCMD:
		return true;

	default:
		return false;
	}
}
