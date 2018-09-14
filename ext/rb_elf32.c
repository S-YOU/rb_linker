#include <stdio.h>
#include "ruby.h"
#include "elf32lib.h"

// For mmap
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

// =============================================================================
// static variables
// =============================================================================
static VALUE rb_elfModule;				// Module ELF
static VALUE rb_cElf32;					// Class Elf32
static VALUE rb_cElf32Sym;				// Class Elf32Sym
static VALUE rb_cElf32Rel;				// Class Elf32Rel
static VALUE rb_cElf32Rela;				// Class Elf32Rela

// =============================================================================
// Elf32Sym
// =============================================================================
static VALUE elf32sym_alloc(VALUE self);
static VALUE rb_elf32sym_new(void);
static size_t rb_elf32sym_size(const void *ptr);
static void rb_elf32sym_free(void *ptr);
static VALUE elf32sym_struct2obj(const Elf32_Sym *pSym);
static VALUE elf32sym_show(VALUE self);
static VALUE elf32sym_get_st_name(VALUE self);
static VALUE elf32sym_set_st_name(VALUE self, VALUE name);
static VALUE elf32sym_get_st_value(VALUE self);
static VALUE elf32sym_set_st_value(VALUE self, VALUE value);
static VALUE elf32sym_get_st_size(VALUE self);
static VALUE elf32sym_set_st_size(VALUE self, VALUE size);
static VALUE elf32sym_get_st_info(VALUE self);
static VALUE elf32sym_set_st_info(VALUE self, VALUE info);
static VALUE elf32sym_get_st_other(VALUE self);
static VALUE elf32sym_set_st_other(VALUE self, VALUE other);
static VALUE elf32sym_get_st_shndx(VALUE self);
static VALUE elf32sym_set_st_shndx(VALUE self, VALUE shndx);

// =============================================================================
// Elf32Rel
// =============================================================================
static VALUE elf32rel_alloc(VALUE self);
static VALUE rb_elf32rel_new(void);
static size_t rb_elf32rel_size(const void *ptr);
static void rb_elf32rel_free(void *ptr);
static VALUE elf32rel_struct2obj(const Elf32_Rel *pRel);
static VALUE elf32rel_get_r_offset(VALUE self);
static VALUE elf32rel_set_r_offset(VALUE self, VALUE offset);
static VALUE elf32rel_get_r_info(VALUE self);
static VALUE elf32rel_set_r_info(VALUE self, VALUE info);

// =============================================================================
// Elf32Rela
// =============================================================================
static VALUE elf32rela_alloc(VALUE self);
static VALUE rb_elf32rela_new(void);
static size_t rb_elf32rela_size(const void *ptr);
static void rb_elf32rela_free(void *ptr);
static VALUE elf32rela_struct2obj(const Elf32_Rela *pRela);
static VALUE elf32rela_get_r_offset(VALUE self);
static VALUE elf32rela_set_r_offset(VALUE self, VALUE offset);
static VALUE elf32rela_get_r_info(VALUE self);
static VALUE elf32rela_set_r_info(VALUE self, VALUE info);
static VALUE elf32rela_get_r_addend(VALUE self);
static VALUE elf32rela_set_r_addend(VALUE self, VALUE addend);

// =============================================================================
// Elf32
// =============================================================================
static VALUE elf32_alloc(VALUE klass);
static void rb_elf32_free(void *ptr);
static size_t rb_elf32_size(const void *ptr);
static VALUE elf32_initialize(VALUE self, VALUE filepath);
static VALUE elf32_show_Ehdr(VALUE self);
static VALUE elf32_get_symtab(VALUE self);
static VALUE elf32_ary2symtab(VALUE self, VALUE ary);
static VALUE elf32_ary2reltab(VALUE self, VALUE ary);
static VALUE elf32_ary2relatab(VALUE self, VALUE ary);

// =============================================================================
// raise exception
// =============================================================================
static void raise_exception(const char *fname, int lnum)
{
	// TODO error handle
	rb_exc_raise(rb_str_new2("TODO Exception!!"));
}

// =============================================================================
// debug printf
// =============================================================================
static void dbg_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "[DEBUG] ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
	return;
}

// =============================================================================
// Elf32Sym type info
// =============================================================================
static const rb_data_type_t rb_elf32sym_type = {
    "ELF/Elf32Sym",
    {
		0,							// dmark
    	rb_elf32sym_free,			// dfree
    	rb_elf32sym_size,			// dsize
    	{0},						// reserved
    },
    0,								// parent
	0,								// for user
	RUBY_TYPED_FREE_IMMEDIATELY,	// free when unused.
};

// =============================================================================
// Elf32Sym alloc
// =============================================================================
static VALUE elf32sym_alloc(VALUE self)
{
	Elf32_Sym *ptr;
	return TypedData_Make_Struct(self, Elf32_Sym, &rb_elf32sym_type, ptr);
}

// =============================================================================
// Elf32Sym new
// =============================================================================
static VALUE rb_elf32sym_new(void)
{
	return elf32sym_alloc(rb_cElf32Sym);
}

// =============================================================================
// Elf32Sym free
// =============================================================================
static void rb_elf32sym_free(void *ptr)
{
	dbg_printf( "%s:%d %s %s", __FILE__, __LINE__, __FUNCTION__, "In..." );
	free(ptr);
	dbg_printf( "%s:%d %s %s", __FILE__, __LINE__, __FUNCTION__, "Out..." );
	return;
}

// =============================================================================
// Elf32Sym size
// =============================================================================
static size_t rb_elf32sym_size(const void *ptr)
{
	return sizeof(Elf32_Sym);
}

// =============================================================================
// Elf32Sym create object from Elf32_Sym
// =============================================================================
static VALUE elf32sym_struct2obj(const Elf32_Sym *pSym)
{
	VALUE obj;
	Elf32_Sym *ptr;
	obj = rb_elf32sym_new();
	TypedData_Get_Struct(obj, Elf32_Sym, &rb_elf32sym_type, ptr);
	memcpy(ptr, pSym, sizeof(Elf32_Sym));
	return obj;
}

// =============================================================================
// Elf32Sym show member values
// =============================================================================
static VALUE elf32sym_show(VALUE self)
{
	Elf32_Sym *pSym;
	dbg_printf( "%s:%d %s %s", __FILE__, __LINE__, __FUNCTION__, "In..." );
	TypedData_Get_Struct(self, Elf32_Sym, &rb_elf32sym_type, pSym);
	fprintf(stdout, "st_name:[%d], ", pSym->st_name);
	fprintf(stdout, "st_value:[%d], ", pSym->st_value);
	fprintf(stdout, "st_size:[%d], ", pSym->st_size);
	fprintf(stdout, "st_info:[%d], ", pSym->st_info);
	fprintf(stdout, "st_other:[%d], ", pSym->st_other);
	fprintf(stdout, "st_shndx:[%d]\n", pSym->st_shndx);
	dbg_printf( "%s:%d %s %s", __FILE__, __LINE__, __FUNCTION__, "Out..." );
	return Qnil;
}

// =============================================================================
// Elf32Sym Get st_name
// =============================================================================
static VALUE elf32sym_get_st_name(VALUE self)
{
	Elf32_Sym *pSym;
	TypedData_Get_Struct(self, Elf32_Sym, &rb_elf32sym_type, pSym);
	return INT2NUM(pSym->st_name);
}

// =============================================================================
// Elf32Sym Set st_name
// =============================================================================
static VALUE elf32sym_set_st_name(VALUE self, VALUE name)
{
	Elf32_Sym *pSym;
	Check_Type(name, T_FIXNUM);
	TypedData_Get_Struct(self, Elf32_Sym, &rb_elf32sym_type, pSym);
	pSym->st_name = NUM2INT(name);
	return self;
}

// =============================================================================
// Elf32Sym Get st_value
// =============================================================================
static VALUE elf32sym_get_st_value(VALUE self)
{
	Elf32_Sym *pSym;
	TypedData_Get_Struct(self, Elf32_Sym, &rb_elf32sym_type, pSym);
	return INT2NUM(pSym->st_value);
}

// =============================================================================
// Elf32Sym Set st_value
// =============================================================================
static VALUE elf32sym_set_st_value(VALUE self, VALUE value)
{
	Elf32_Sym *pSym;
	Check_Type(value, T_FIXNUM);
	TypedData_Get_Struct(self, Elf32_Sym, &rb_elf32sym_type, pSym);
	pSym->st_value = NUM2INT(value);
	return self;
}

// =============================================================================
// Elf32Sym Get st_size
// =============================================================================
static VALUE elf32sym_get_st_size(VALUE self)
{
	Elf32_Sym *pSym;
	TypedData_Get_Struct(self, Elf32_Sym, &rb_elf32sym_type, pSym);
	return INT2NUM(pSym->st_size);
}

// =============================================================================
// Elf32Sym Set st_size
// =============================================================================
static VALUE elf32sym_set_st_size(VALUE self, VALUE size)
{
	Elf32_Sym *pSym;
	Check_Type(size, T_FIXNUM);
	TypedData_Get_Struct(self, Elf32_Sym, &rb_elf32sym_type, pSym);
	pSym->st_size = NUM2INT(size);
	return self;
}

// =============================================================================
// Elf32Sym Get st_info
// =============================================================================
static VALUE elf32sym_get_st_info(VALUE self)
{
	Elf32_Sym *pSym;
	TypedData_Get_Struct(self, Elf32_Sym, &rb_elf32sym_type, pSym);
	return INT2NUM(pSym->st_info);
}

// =============================================================================
// Elf32Sym Set st_info
// =============================================================================
static VALUE elf32sym_set_st_info(VALUE self, VALUE info)
{
	Elf32_Sym *pSym;
	Check_Type(info, T_FIXNUM);
	TypedData_Get_Struct(self, Elf32_Sym, &rb_elf32sym_type, pSym);
	pSym->st_info = NUM2INT(info);
	return self;
}

// =============================================================================
// Elf32Sym Get st_other
// =============================================================================
static VALUE elf32sym_get_st_other(VALUE self)
{
	Elf32_Sym *pSym;
	TypedData_Get_Struct(self, Elf32_Sym, &rb_elf32sym_type, pSym);
	return INT2NUM(pSym->st_other);
}

// =============================================================================
// Elf32Sym Set st_other
// =============================================================================
static VALUE elf32sym_set_st_other(VALUE self, VALUE other)
{
	Elf32_Sym *pSym;
	Check_Type(other, T_FIXNUM);
	TypedData_Get_Struct(self, Elf32_Sym, &rb_elf32sym_type, pSym);
	pSym->st_other = NUM2INT(other);
	return self;
}

// =============================================================================
// Elf32Sym Get st_shndx
// =============================================================================
static VALUE elf32sym_get_st_shndx(VALUE self)
{
	Elf32_Sym *pSym;
	TypedData_Get_Struct(self, Elf32_Sym, &rb_elf32sym_type, pSym);
	return INT2NUM(pSym->st_shndx);
}

// =============================================================================
// Elf32Sym Set st_shndx
// =============================================================================
static VALUE elf32sym_set_st_shndx(VALUE self, VALUE shndx)
{
	Elf32_Sym *pSym;
	Check_Type(shndx, T_FIXNUM);
	TypedData_Get_Struct(self, Elf32_Sym, &rb_elf32sym_type, pSym);
	pSym->st_shndx = NUM2INT(shndx);
	return self;
}

// =============================================================================
// Elf32Rel type info
// =============================================================================
static const rb_data_type_t rb_elf32rel_type = {
    "ELF/Elf32Rel",
    {
		0,							// dmark
    	rb_elf32rel_free,			// dfree
    	rb_elf32rel_size,			// dsize
    	{0},						// reserved
    },
    0,								// parent
	0,								// for user
	RUBY_TYPED_FREE_IMMEDIATELY,	// free when unused.
};

// =============================================================================
// Elf32Rel alloc
// =============================================================================
static VALUE elf32rel_alloc(VALUE self)
{
	Elf32_Rel *ptr;
	return TypedData_Make_Struct(self, Elf32_Rel, &rb_elf32rel_type, ptr);
}

// =============================================================================
// Elf32Rel new
// =============================================================================
static VALUE rb_elf32rel_new(void)
{
	return elf32rel_alloc(rb_cElf32Rel);
}

// =============================================================================
// Elf32Rel free
// =============================================================================
static void rb_elf32rel_free(void *ptr)
{
	dbg_printf( "%s:%d %s %s", __FILE__, __LINE__, __FUNCTION__, "In..." );
	free(ptr);
	dbg_printf( "%s:%d %s %s", __FILE__, __LINE__, __FUNCTION__, "Out..." );
	return;
}

// =============================================================================
// Elf32Rel size
// =============================================================================
static size_t rb_elf32rel_size(const void *ptr)
{
	return sizeof(Elf32_Rel);
}

// =============================================================================
// Elf32Rel create object from Elf32_Rel
// =============================================================================
static VALUE elf32rel_struct2obj(const Elf32_Rel *pRel)
{
	VALUE obj;
	Elf32_Rel *ptr;
	obj = rb_elf32rel_new();
	TypedData_Get_Struct(obj, Elf32_Rel, &rb_elf32rel_type, ptr);
	memcpy(ptr, pRel, sizeof(Elf32_Rel));
	return obj;
}

// =============================================================================
// Elf32Rel Get r_offset
// =============================================================================
static VALUE elf32rel_get_r_offset(VALUE self)
{
	Elf32_Rel *pRel;
	TypedData_Get_Struct(self, Elf32_Rel, &rb_elf32rel_type, pRel);
	return INT2NUM(pRel->r_offset);
}

// =============================================================================
// Elf32Rel Set r_offset
// =============================================================================
static VALUE elf32rel_set_r_offset(VALUE self, VALUE offset)
{
	Elf32_Rel *pRel;
	Check_Type(offset, T_FIXNUM);
	TypedData_Get_Struct(self, Elf32_Rel, &rb_elf32rel_type, pRel);
	pRel->r_offset = NUM2INT(offset);
	return self;
}

// =============================================================================
// Elf32Rel Get r_info
// =============================================================================
static VALUE elf32rel_get_r_info(VALUE self)
{
	Elf32_Rel *pRel;
	TypedData_Get_Struct(self, Elf32_Rel, &rb_elf32rel_type, pRel);
	return INT2NUM(pRel->r_info);
}

// =============================================================================
// Elf32Rel Set r_info
// =============================================================================
static VALUE elf32rel_set_r_info(VALUE self, VALUE info)
{
	Elf32_Rel *pRel;
	Check_Type(info, T_FIXNUM);
	TypedData_Get_Struct(self, Elf32_Rel, &rb_elf32rel_type, pRel);
	pRel->r_info = NUM2INT(info);
	return self;
}


// =============================================================================
// Elf32Rela type info
// =============================================================================
static const rb_data_type_t rb_elf32rela_type = {
    "ELF/Elf32Rela",
    {
		0,							// dmark
    	rb_elf32rela_free,			// dfree
    	rb_elf32rela_size,			// dsize
    	{0},						// reserved
    },
    0,								// parent
	0,								// for user
	RUBY_TYPED_FREE_IMMEDIATELY,	// free when unused.
};

// =============================================================================
// Elf32Rela alloc
// =============================================================================
static VALUE elf32rela_alloc(VALUE self)
{
	Elf32_Rela *ptr;
	return TypedData_Make_Struct(self, Elf32_Rela, &rb_elf32rela_type, ptr);
}

// =============================================================================
// Elf32Rela new
// =============================================================================
static VALUE rb_elf32rela_new(void)
{
	return elf32rela_alloc(rb_cElf32Rela);
}

// =============================================================================
// Elf32Rela free
// =============================================================================
static void rb_elf32rela_free(void *ptr)
{
	dbg_printf( "%s:%d %s %s", __FILE__, __LINE__, __FUNCTION__, "In..." );
	free(ptr);
	dbg_printf( "%s:%d %s %s", __FILE__, __LINE__, __FUNCTION__, "Out..." );
	return;
}

// =============================================================================
// Elf32Rela size
// =============================================================================
static size_t rb_elf32rela_size(const void *ptr)
{
	return sizeof(Elf32_Rela);
}

// =============================================================================
// Elf32Rela create object from Elf32_Rela
// =============================================================================
static VALUE elf32rela_struct2obj(const Elf32_Rela *pRela)
{
	VALUE obj;
	Elf32_Rela *ptr;
	obj = rb_elf32rela_new();
	TypedData_Get_Struct(obj, Elf32_Rela, &rb_elf32rela_type, ptr);
	memcpy(ptr, pRela, sizeof(Elf32_Rela));
	return obj;
}

// =============================================================================
// Elf32Rela Get r_offset
// =============================================================================
static VALUE elf32rela_get_r_offset(VALUE self)
{
	Elf32_Rela *pRela;
	TypedData_Get_Struct(self, Elf32_Rela, &rb_elf32rela_type, pRela);
	return INT2NUM(pRela->r_offset);
}

// =============================================================================
// Elf32Rela Set r_offset
// =============================================================================
static VALUE elf32rela_set_r_offset(VALUE self, VALUE offset)
{
	Elf32_Rela *pRela;
	Check_Type(offset, T_FIXNUM);
	TypedData_Get_Struct(self, Elf32_Rela, &rb_elf32rela_type, pRela);
	pRela->r_offset = NUM2INT(offset);
	return self;
}

// =============================================================================
// Elf32Rela Get r_info
// =============================================================================
static VALUE elf32rela_get_r_info(VALUE self)
{
	Elf32_Rela *pRela;
	TypedData_Get_Struct(self, Elf32_Rela, &rb_elf32rela_type, pRela);
	return INT2NUM(pRela->r_info);
}

// =============================================================================
// Elf32Rela Set r_info
// =============================================================================
static VALUE elf32rela_set_r_info(VALUE self, VALUE info)
{
	Elf32_Rela *pRela;
	Check_Type(info, T_FIXNUM);
	TypedData_Get_Struct(self, Elf32_Rela, &rb_elf32rela_type, pRela);
	pRela->r_info = NUM2INT(info);
	return self;
}

// =============================================================================
// Elf32Rela Get r_info
// =============================================================================
static VALUE elf32rela_get_r_addend(VALUE self)
{
	Elf32_Rela *pRela;
	TypedData_Get_Struct(self, Elf32_Rela, &rb_elf32rela_type, pRela);
	return INT2NUM(pRela->r_addend);
}

// =============================================================================
// Elf32Rela Set r_info
// =============================================================================
static VALUE elf32rela_set_r_addend(VALUE self, VALUE addend)
{
	Elf32_Rela *pRela;
	Check_Type(addend, T_FIXNUM);
	TypedData_Get_Struct(self, Elf32_Rela, &rb_elf32rela_type, pRela);
	pRela->r_addend = NUM2INT(addend);
	return self;
}

// =============================================================================
// ELF32 data type
// =============================================================================
static const rb_data_type_t rb_elf32_type = {
    "ELF/Elf32",
    {
		0,							// dmark
    	rb_elf32_free,				// dfree
    	rb_elf32_size,				// dsize
    	{0},						// reserved
    },
    0,								// parent
	0,								// for user
	RUBY_TYPED_FREE_IMMEDIATELY,	// free when unused.
};

// =============================================================================
// ELF32 alloc
// =============================================================================
static VALUE elf32_alloc(VALUE klass)
{
	ST_ELF32 *ptr;
	return TypedData_Make_Struct(klass, ST_ELF32, &rb_elf32_type, ptr);
}

// =============================================================================
// ELF32 free
// =============================================================================
static void rb_elf32_free(void *ptr)
{
	dbg_printf( "%s:%d %s %s", __FILE__, __LINE__, __FUNCTION__, "In..." );

	ST_ELF32 *pElf32 = (ST_ELF32 *)ptr;
    elf32_munmapFile(pElf32, pElf32->length);
	free(pElf32);
	dbg_printf( "%s:%d %s %s", __FILE__, __LINE__, __FUNCTION__, "Out..." );
	return;
}

// =============================================================================
// ELF32 size
// =============================================================================
static size_t rb_elf32_size(const void *ptr)
{
	ST_ELF32 *pElf32 = (ST_ELF32 *)ptr;
	return sizeof(ST_ELF32) + pElf32->length;
}

// =============================================================================
// ELF32 Constructor
// =============================================================================
static VALUE elf32_initialize(VALUE self, VALUE filepath)
{
	int ret;
	ST_ELF32 *pElf32;
	Check_Type( filepath, T_STRING );

	TypedData_Get_Struct(self, ST_ELF32, &rb_elf32_type, pElf32);
	ret = elf32_mmapFile(StringValuePtr(filepath), PROT_READ, MAP_PRIVATE, 0, (void *)&pElf32->pAddr, &pElf32->length);
	if (ret < 0) {
		raise_exception(__FUNCTION__, __LINE__);
	}
	return self;
}

// =============================================================================
// ELF32 dump ELF Header
// =============================================================================
static VALUE elf32_show_Ehdr(VALUE self)
{
	ST_ELF32 *pElf32;
	TypedData_Get_Struct(self, ST_ELF32, &rb_elf32_type, pElf32);
	elf32_showEhdr( (Elf32_Ehdr *)pElf32->pAddr );
	return Qnil;
}

// =============================================================================
// ELF32 Get symbol table (Elf32_Sym list)
// =============================================================================
static VALUE elf32_get_symtab(VALUE self)
{
	int ret;
	VALUE ary = Qnil;
	ST_ELF32 *pSelf;
	Elf32_Shdr *pShdr;
	uint32_t idx;
	Elf32_Sym *pSymtab;
	size_t size;
	char *pSymtabName;

	dbg_printf( "%s:%d %s %s", __FILE__, __LINE__, __FUNCTION__, "In..." );
	#if 0
	if (!NIL_P(name)) {
		pSymtabName = ".symtab";
	} else {
		pSymtabName = StringValuePtr(name);
	}
	#endif
	pSymtabName = ".symtab";

	TypedData_Get_Struct(self, ST_ELF32, &rb_elf32_type, pSelf);
	ary = rb_ary_new();
	ret = elf32_searchShdr(pSelf->pAddr, pSymtabName, &pShdr, &idx);
	if (ret < 0) {
		// TODO 
		// Not Found
		return Qnil;
	}

	pSymtab = (Elf32_Sym *)(pSelf->pAddr + pShdr->sh_offset);
	size = pShdr->sh_size;
	while(0 < size)
	{
		rb_ary_push(ary, elf32sym_struct2obj(pSymtab));
		pSymtab++;
		size -= sizeof(Elf32_Sym);
	}

	dbg_printf( "%s:%d %s %s", __FILE__, __LINE__, __FUNCTION__, "In..." );
	return ary;
}

// =============================================================================
// ELF32 Create Elf32_Sym Array from Array(Byte Array)
// =============================================================================
static VALUE elf32_ary2symtab(VALUE self, VALUE ary)
{
	int i,len;
	int count;
	len = RARRAY_LEN(ary);
	uint8_t *pBin;
	Elf32_Sym *pSym;
	VALUE symtab;

	Check_Type( ary, T_ARRAY );

	pBin = malloc(len);
	for(i = 0; i < len; i++) {
		pBin[i] = NUM2CHR(rb_ary_entry(ary, i));
	}

	pSym = (Elf32_Sym *)pBin;
	symtab = rb_ary_new();
	count = len / sizeof(Elf32_Sym);
	for (i = 0; i < count; i++) {
		rb_ary_push(symtab, elf32sym_struct2obj(pSym));
		pSym++;
	}
	return symtab;
}

// =============================================================================
// ELF32 Create Elf32_Rel Array from Array(Byte Array)
// =============================================================================
static VALUE elf32_ary2reltab(VALUE self, VALUE ary)
{
	int i,len, count;
	len = RARRAY_LEN(ary);
	uint8_t *pBin;
	Elf32_Rel *pRel;
	VALUE reltab;

	Check_Type( ary, T_ARRAY );

	pBin = malloc(len);
	for(i = 0; i < len; i++) {
		pBin[i] = NUM2CHR(rb_ary_entry(ary, i));
	}

	pRel = (Elf32_Rel *)pBin;
	reltab = rb_ary_new();
	count = len / sizeof(Elf32_Rel);
	for (i = 0; i < count; i++) {
		rb_ary_push(reltab, elf32rel_struct2obj(pRel));
		pRel++;
	}
	return reltab;
}

// =============================================================================
// ELF32 Create Elf32_Rel Array from Array(Byte Array)
// =============================================================================
static VALUE elf32_ary2relatab(VALUE self, VALUE ary)
{
	int i,len, count;
	len = RARRAY_LEN(ary);
	uint8_t *pBin;
	Elf32_Rela *pRela;
	VALUE relatab;

	Check_Type( ary, T_ARRAY );

	pBin = malloc(len);
	for(i = 0; i < len; i++) {
		pBin[i] = NUM2CHR(rb_ary_entry(ary, i));
	}

	pRela = (Elf32_Rela *)pBin;
	relatab = rb_ary_new();
	count = len / sizeof(Elf32_Rela);
	for (i = 0; i < count; i++) {
		rb_ary_push(relatab, elf32rela_struct2obj(pRela));
		pRela++;
	}
	return relatab;
}

// =============================================================================
// Initialize ext Module
// =============================================================================
void Init_elf32( void ) {

	// define ELF module
	rb_elfModule = rb_define_module("ELF");

	// Initialize rb_cElf32
	rb_cElf32 = rb_define_class_under(rb_elfModule, "Elf32" , rb_cObject);
    rb_define_alloc_func(rb_cElf32, elf32_alloc);
	rb_define_method(rb_cElf32, "initialize", elf32_initialize, 1);
	rb_define_method(rb_cElf32, "show_Ehdr", elf32_show_Ehdr, 0);
	rb_define_method(rb_cElf32, "symtab", elf32_get_symtab, 0);
	rb_define_singleton_method( rb_cElf32, "to_symtab", elf32_ary2symtab, 1);
	rb_define_singleton_method( rb_cElf32, "to_reltab", elf32_ary2reltab, 1);
	rb_define_singleton_method( rb_cElf32, "to_relatab", elf32_ary2relatab, 1);

	// Initialize rb_cElf32Sym
	rb_cElf32Sym = rb_define_class_under(rb_elfModule, "Elf32Sym" , rb_cObject);
    rb_define_alloc_func(rb_cElf32Sym, elf32sym_alloc);
	rb_define_method(rb_cElf32Sym, "show", elf32sym_show, 0);
	rb_define_method(rb_cElf32Sym, "st_name", elf32sym_get_st_name, 0);
	rb_define_method(rb_cElf32Sym, "st_name=", elf32sym_set_st_name, 1);
	rb_define_method(rb_cElf32Sym, "st_value", elf32sym_get_st_value, 0);
	rb_define_method(rb_cElf32Sym, "st_value=", elf32sym_set_st_value, 1);
	rb_define_method(rb_cElf32Sym, "st_size", elf32sym_get_st_size, 0);
	rb_define_method(rb_cElf32Sym, "st_size=", elf32sym_set_st_size, 1);
	rb_define_method(rb_cElf32Sym, "st_info", elf32sym_get_st_info, 0);
	rb_define_method(rb_cElf32Sym, "st_info=", elf32sym_set_st_info, 1);
	rb_define_method(rb_cElf32Sym, "st_other", elf32sym_get_st_other, 0);
	rb_define_method(rb_cElf32Sym, "st_other=", elf32sym_set_st_other, 1);
	rb_define_method(rb_cElf32Sym, "st_shndx", elf32sym_get_st_shndx, 0);
	rb_define_method(rb_cElf32Sym, "st_shndx=", elf32sym_set_st_shndx, 1);

	// Initialize rb_cElf32Rel
	rb_cElf32Rel = rb_define_class_under(rb_elfModule, "Elf32Rel" , rb_cObject);
    rb_define_alloc_func(rb_cElf32Rel, elf32rel_alloc);
	rb_define_method(rb_cElf32Rel, "r_offset", elf32rel_get_r_offset, 0);
	rb_define_method(rb_cElf32Rel, "r_offset=", elf32rel_set_r_offset, 1);
	rb_define_method(rb_cElf32Rel, "r_info", elf32rel_get_r_info, 0);
	rb_define_method(rb_cElf32Rel, "r_info=", elf32rel_set_r_info, 1);

	// Initialize rb_cElf32Rela
	rb_cElf32Rela = rb_define_class_under(rb_elfModule, "Elf32Rela" , rb_cObject);
    rb_define_alloc_func(rb_cElf32Rela, elf32rela_alloc);
	rb_define_method(rb_cElf32Rela, "r_offset", elf32rela_get_r_offset, 0);
	rb_define_method(rb_cElf32Rela, "r_offset=", elf32rela_set_r_offset, 1);
	rb_define_method(rb_cElf32Rela, "r_info", elf32rela_get_r_info, 0);
	rb_define_method(rb_cElf32Rela, "r_info=", elf32rela_set_r_info, 1);
	rb_define_method(rb_cElf32Rela, "r_addend", elf32rela_get_r_addend, 0);
	rb_define_method(rb_cElf32Rela, "r_addend=", elf32rela_set_r_addend, 1);
	return;
}

// 実装方針
// とりあえずシンボルテーブルを全部くっつける
// タイプを気にせず関連する文字列をコピー
// →.strtabセクション内でのオフセットは更新する必要がある
// →先に.strtabを結合し、オブジェクトのオフセット値を保持しておく
// .symtabを最後に結合し、オフセット値を足して文字列位置を補正する
// シンボルが必要になる理由
// リロケーションの時にシンボルテーブルのインデックスで指定されるので
// 結合できていないとリロケーションができない
// 順番
// .strtabのマージ→オブジェクト全体を更新する


// オブジェクト単位でオフセットを保持し、セクション内のオフセットを更新する。
