#ifndef BoyiaAsmUtils_h
#define BoyiaAsmUtils_h

inline bool is_uintn(int x, int n) {
	return (x & -(1 << n)) == 0;
}

inline bool is_uint2(int x) { return is_uintn(x, 2); }
inline bool is_uint3(int x) { return is_uintn(x, 3); }
inline bool is_uint4(int x) { return is_uintn(x, 4); }
inline bool is_uint5(int x) { return is_uintn(x, 5); }
inline bool is_uint6(int x) { return is_uintn(x, 6); }
inline bool is_uint8(int x) { return is_uintn(x, 8); }
inline bool is_uint10(int x) { return is_uintn(x, 10); }
inline bool is_uint12(int x) { return is_uintn(x, 12); }
inline bool is_uint16(int x) { return is_uintn(x, 16); }
inline bool is_uint24(int x) { return is_uintn(x, 24); }
inline bool is_uint26(int x) { return is_uintn(x, 26); }
inline bool is_uint28(int x) { return is_uintn(x, 28); }

inline bool is_intn(int x, int n) {
	return -(1 << (n - 1)) <= x && x < (1 << (n - 1));
}

inline bool is_int8(int x) { return is_intn(x, 8); }
inline bool is_int16(int x) { return is_intn(x, 16); }
inline bool is_int18(int x) { return is_intn(x, 18); }
inline bool is_int24(int x) { return is_intn(x, 24); }


#endif // !Assembler_h
