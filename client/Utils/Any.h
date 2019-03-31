#pragma once
#include "../NovusTypes.h"

// WARNING: Please consult with Pursche before using this, it's ugly and only used in scripts for unrolling a variadic template argument list
class any {
public:
	enum type { U8, U16, U32, U64, I8, I16, I32, I64, F32, F64, Bool, String };
	any(u8 e) { m_data.U8 = e; m_type = U8; }
	any(u16 e) { m_data.U16 = e; m_type = U16; }
	any(u32 e) { m_data.U32 = e; m_type = U32; }
	any(u64 e) { m_data.U64 = e; m_type = U64; }
	any(i8 e) { m_data.I8 = e; m_type = I8; }
	any(i16 e) { m_data.I16 = e; m_type = I16; }
	any(i32 e) { m_data.I32 = e; m_type = I32; }
	any(i64 e) { m_data.I64 = e; m_type = I64; }
	any(f32 e) { m_data.F32 = e; m_type = F32; }
	any(f64 e) { m_data.F64 = e; m_type = F64; }
	any(bool e) { m_data.BOOL = e; m_type = Bool; }
	//any(std::string e) { m_data.STRING = e; m_type = String; }

	type get_type() const { return m_type; }
	u8 get_u8() const { return m_data.U8; }
	u16 get_u16() const { return m_data.U16; }
	u32 get_u32() const { return m_data.U32; }
	u64 get_u64() const { return m_data.U64; }
	i8 get_i8() const { return m_data.I8; }
	i16 get_i16() const { return m_data.I16; }
	i32 get_i32() const { return m_data.I32; }
	i64 get_i64() const { return m_data.I64; }
	f32 get_f32() const { return m_data.F32; }
	f64 get_f64() const { return m_data.F64; }
	bool get_bool() const { return m_data.BOOL; }
	//std::string get_string() const { return m_data.STRING; }

private:
	type m_type;
	union {
		u8	U8;
		u16	U16;
		u32	U32;
		u64	U64;
		i8	I8;
		i16	I16;
		i32	I32;
		i64	I64;
		f32 F32;
		f64 F64;
		bool BOOL;
		//std::string STRING;
	} m_data;
};