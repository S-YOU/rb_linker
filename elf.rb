require './monkey_patch'
require './machine_arch_list'

class ELF
	# ============================================================================
	# ELF Identifer
	# ============================================================================
	ELF_SIZE_HALF_WORD	= 2
	ELF_SIZE_WORD				= 4
	ELF_SIZE_XWORD			= 8
	ELF_SIZE_ADDR_32		= 4
	ELF_SIZE_ADDR_64		= 8
	ELF_SIZE_OFFSET_32	= 4
	ELF_SIZE_OFFSET_64	= 8

	# ============================================================================
	# ELF Identifer
	# ============================================================================
	ELF_IDENT_SIZE	                  = 16
	ELF_IDENT_OFFSET_MAGIC_NUMBER 		= 0
	ELF_IDENT_OFFSET_CLASS						= 4
	ELF_IDENT_OFFSET_ENDIAN						= 5
	ELF_IDENT_OFFSET_FORMAT_VERSION  	= 6
	ELF_IDENT_OFFSET_OS_ABI				  	= 7
	ELF_IDENT_OFFSET_OS_ABI_VERSION 	= 8

	# ============================================================================
	# ELF32
	# ============================================================================
	ELF32_OFFSET_IDENTIFER   					= 0
	ELF32_OFFSET_TYPE        					= ELF32_OFFSET_IDENTIFER 						+ ELF_IDENT_SIZE
	ELF32_OFFSET_MACHINE     					= ELF32_OFFSET_TYPE 				    		+ ELF_SIZE_HALF_WORD
	ELF32_OFFSET_VERSION     					= ELF32_OFFSET_MACHINE 				 			+ ELF_SIZE_HALF_WORD
	ELF32_OFFSET_ENTRY       					= ELF32_OFFSET_VERSION 							+ ELF_SIZE_WORD
	ELF32_OFFSET_PROGRAM_HEADER	 			= ELF32_OFFSET_ENTRY 		  					+ ELF_SIZE_ADDR_32
	ELF32_OFFSET_SECTION_HEADER 			= ELF32_OFFSET_PROGRAM_HEADER 			+ ELF_SIZE_OFFSET_32
	ELF32_OFFSET_FLAGS								= ELF32_OFFSET_SECTION_HEADER				+ ELF_SIZE_OFFSET_32
	ELF32_OFFSET_ELF_HEADER_SIZE			= ELF32_OFFSET_FLAGS 								+ ELF_SIZE_WORD
	ELF32_OFFSET_PROGRAM_HEADER_SIZE  = ELF32_OFFSET_ELF_HEADER_SIZE		 	+ ELF_SIZE_HALF_WORD
	ELF32_OFFSET_PROGRAM_HEADER_NUM 	= ELF32_OFFSET_PROGRAM_HEADER_SIZE  + ELF_SIZE_HALF_WORD
	ELF32_OFFSET_SECTION_HEADER_SIZE 	= ELF32_OFFSET_PROGRAM_HEADER_NUM 	+ ELF_SIZE_HALF_WORD
	ELF32_OFFSET_SECTION_HEADER_NUM 	= ELF32_OFFSET_SECTION_HEADER_SIZE 	+ ELF_SIZE_HALF_WORD
	ELF32_OFFSET_SECTION_NAME_IDX 	  = ELF32_OFFSET_SECTION_HEADER_NUM 	+ ELF_SIZE_HALF_WORD

	# ============================================================================
	# ELF64
	# ============================================================================
	ELF64_OFFSET_IDENTIFER   					= 0
	ELF64_OFFSET_TYPE        					= ELF64_OFFSET_IDENTIFER 						+ ELF_IDENT_SIZE
	ELF64_OFFSET_MACHINE     					= ELF64_OFFSET_TYPE 				    		+ ELF_SIZE_HALF_WORD
	ELF64_OFFSET_VERSION     					= ELF64_OFFSET_MACHINE 				 			+ ELF_SIZE_HALF_WORD
	ELF64_OFFSET_ENTRY       					= ELF64_OFFSET_VERSION 							+ ELF_SIZE_WORD
	ELF64_OFFSET_PROGRAM_HEADER	 			= ELF64_OFFSET_ENTRY 		  					+ ELF_SIZE_ADDR_64
	ELF64_OFFSET_SECTION_HEADER 			= ELF64_OFFSET_PROGRAM_HEADER 			+ ELF_SIZE_OFFSET_64
	ELF64_OFFSET_FLAGS								= ELF64_OFFSET_SECTION_HEADER				+ ELF_SIZE_OFFSET_64
	ELF64_OFFSET_ELF_HEADER_SIZE			= ELF64_OFFSET_FLAGS 								+ ELF_SIZE_WORD
	ELF64_OFFSET_PROGRAM_HEADER_SIZE  = ELF64_OFFSET_ELF_HEADER_SIZE		 	+ ELF_SIZE_HALF_WORD
	ELF64_OFFSET_PROGRAM_HEADER_NUM 	= ELF64_OFFSET_PROGRAM_HEADER_SIZE  + ELF_SIZE_HALF_WORD
	ELF64_OFFSET_SECTION_HEADER_SIZE 	= ELF64_OFFSET_PROGRAM_HEADER_NUM 	+ ELF_SIZE_HALF_WORD
	ELF64_OFFSET_SECTION_HEADER_NUM 	= ELF64_OFFSET_SECTION_HEADER_SIZE 	+ ELF_SIZE_HALF_WORD
	ELF64_OFFSET_SECTION_NAME_IDX 	  = ELF64_OFFSET_SECTION_HEADER_NUM 	+ ELF_SIZE_HALF_WORD

	# ============================================================================
	# Legal values for e_type (object file type)
	# ============================================================================
	ELF_ET_NONE	= 0
	ELF_ET_REL	= 1
 	ELF_ET_EXEC = 2
	ELF_ET_DYN	= 3
	ELF_ET_CORE	= 4

	# ============================================================================
	# Construster
	# - Load Object File to memory.
	# ============================================================================
	def initialize filepath
		load(filepath)
	end

	# ============================================================================
	# Load Object File
	# - Check if valid ELF and set elf infos.
	# ============================================================================
	def load filepath
		bin = File.binread(filepath).unpack("C*")
		elf_ident = bin[0, ELF_IDENT_SIZE]

		# check magic number
		unless is_elf? elf_ident
			throw "This is not ELF Format File"
		end

		# Check ELF class
		val = elf_ident[ELF_IDENT_OFFSET_CLASS].ord
		case val
		when 1
			@elf_class = :CLASS_ELF32
		when 2
			@elf_class = :CLASS_ELF64
		else
			throw "Invalid ELF Class:#{val}"
		end

		# Check Endian
		val = elf_ident[ELF_IDENT_OFFSET_ENDIAN].ord
		case val
		when 1
			@elf_endian = :ELF_LITTLE_ENDIAN
		when 2
			@elf_endian = :ELF_BIG_ENDIAN
		else
			throw "Invalid ELF Endian:#{val}"
		end

		# Check ELF Format Version
		val = elf_ident[ELF_IDENT_OFFSET_FORMAT_VERSION].ord
		unless val == 1
			throw "Unsuppoted ELF Format Version:#{val}"
		end
		@elf_version = val

		# Check OS ABI
		val = elf_ident[ELF_IDENT_OFFSET_OS_ABI].ord
		case val
		when 0
			@os_abi = :OS_ABI_UNIX
		when 3
			@os_abi = :OS_ABI_LINUX
		else
			throw "Unsuppoted OS ABI Format:#{val}"
		end

		# Check OS ABI Version
		@os_abi_version = elf_ident[ELF_IDENT_OFFSET_OS_ABI_VERSION]

		@bin = bin
		@ident = elf_ident

		is_little = @elf_endian == :ELF_LITTLE_ENDIAN
		case @elf_class
		when :CLASS_ELF32
			@elf_type             = @bin[ELF32_OFFSET_TYPE, ELF_SIZE_HALF_WORD].to_i(is_little)
			@elf_machine          = @bin[ELF32_OFFSET_MACHINE, ELF_SIZE_HALF_WORD].to_i(is_little)
			@elf_version          = @bin[ELF32_OFFSET_VERSION, ELF_SIZE_HALF_WORD].to_i(is_little)
			@elf_entry            = @bin[ELF32_OFFSET_ENTRY, ELF_SIZE_ADDR_32].to_i(is_little)
			@elf_program_h_offset = @bin[ELF32_OFFSET_PROGRAM_HEADER, ELF_SIZE_OFFSET_32].to_i(is_little)
			@elf_section_h_offset = @bin[ELF32_OFFSET_SECTION_HEADER, ELF_SIZE_OFFSET_32].to_i(is_little)
			@elf_flags            = @bin[ELF32_OFFSET_FLAGS, ELF_SIZE_WORD].to_i(is_little)
			@elf_h_size       		= @bin[ELF32_OFFSET_ELF_HEADER_SIZE, ELF_SIZE_HALF_WORD].to_i(is_little)
			@elf_program_h_size   = @bin[ELF32_OFFSET_PROGRAM_HEADER_SIZE, ELF_SIZE_HALF_WORD].to_i(is_little)
			@elf_program_h_num    = @bin[ELF32_OFFSET_PROGRAM_HEADER_NUM, ELF_SIZE_HALF_WORD].to_i(is_little)
			@elf_section_h_size   = @bin[ELF32_OFFSET_SECTION_HEADER_SIZE, ELF_SIZE_HALF_WORD].to_i(is_little)
			@elf_section_h_num    = @bin[ELF32_OFFSET_SECTION_HEADER_NUM, ELF_SIZE_HALF_WORD].to_i(is_little)
			@elf_section_name_idx = @bin[ELF32_OFFSET_SECTION_NAME_IDX, ELF_SIZE_HALF_WORD].to_i(is_little)
		when :CLASS_ELF64
			@elf_type             = @bin[ELF64_OFFSET_TYPE, ELF_SIZE_HALF_WORD].to_i(is_little)
			@elf_machine          = @bin[ELF64_OFFSET_MACHINE, ELF_SIZE_HALF_WORD].to_i(is_little)
			@elf_version          = @bin[ELF64_OFFSET_VERSION, ELF_SIZE_HALF_WORD].to_i(is_little)
			@elf_entry            = @bin[ELF64_OFFSET_ENTRY, ELF_SIZE_ADDR_64].to_i(is_little)
			@elf_program_h_offset = @bin[ELF64_OFFSET_PROGRAM_HEADER, ELF_SIZE_OFFSET_64].to_i(is_little)
			@elf_section_h_offset = @bin[ELF64_OFFSET_SECTION_HEADER, ELF_SIZE_OFFSET_64].to_i(is_little)
			@elf_flags            = @bin[ELF64_OFFSET_FLAGS, ELF_SIZE_WORD].to_i(is_little)
			@elf_h_size       		= @bin[ELF64_OFFSET_ELF_HEADER_SIZE, ELF_SIZE_HALF_WORD].to_i(is_little)
			@elf_program_h_size   = @bin[ELF64_OFFSET_PROGRAM_HEADER_SIZE, ELF_SIZE_HALF_WORD].to_i(is_little)
			@elf_program_h_num    = @bin[ELF64_OFFSET_PROGRAM_HEADER_NUM, ELF_SIZE_HALF_WORD].to_i(is_little)
			@elf_section_h_size   = @bin[ELF64_OFFSET_SECTION_HEADER_SIZE, ELF_SIZE_HALF_WORD].to_i(is_little)
			@elf_section_h_num    = @bin[ELF64_OFFSET_SECTION_HEADER_NUM, ELF_SIZE_HALF_WORD].to_i(is_little)
			@elf_section_name_idx = @bin[ELF64_OFFSET_SECTION_NAME_IDX, ELF_SIZE_HALF_WORD].to_i(is_little)
		else
			throw "Invalid ELF Class #{@elf_class}"
		end

		# create section name - section index map.
		initialize_section_h_map
	end

	# ============================================================================
	# Get section header info by section name
	# ============================================================================
	def get_section_header section_name
		sec_idx = @sh_idx_map[section_name]

		# No such section
		return nil if sec_idx.nil?

		sec_pos = @e_shoff + (sec_idx * @sh_size)
		@bin[sec_pos, @sh_size]
	end

	# ============================================================================
	# Show ELF Header info like `readelf -h` format.
	# ============================================================================
	def show_elf_header
		puts "ELF Header:"
		show_magic
		show_elf_class
		show_endian
		show_elf_version
		show_OS_ABI
		show_ABI_version
		show_file_type
		show_machine_arch
		show_file_version
		show_entry_point
		show_program_h_offset
		show_section_h_offset
		show_elf_flags
		show_elf_h_size
		show_program_h_size
		show_program_h_num
		show_section_h_size
		show_section_h_num
		show_section_name_idx

# TODO
#		show_sections
#		initialize_section_idx_map
#		debug_section = get_section_header ".debug_info"
#		show_section_header debug_section
	end

	# ============================================================================
	# Show Section Header Info
	# ============================================================================
	def show_section_header section_header
	end

	# ============================================================================
	# Show ELF Magic Number
	# ============================================================================
	def show_magic
		puts "  Magic:   #{@ident.hex_dump(false, false)}"
	end

	# ============================================================================
	# Show ELF Class Info
	# ============================================================================
	def show_elf_class
		class_str = ""
		case @elf_class
		when :CLASS_ELF32
			class_str = "ELF32"
		when :CLASS_ELF64
			class_str = "ELF64"
		else
			class_str = "Invalid Class"
		end
		puts "  Class:                             #{class_str}"
	end

	# ============================================================================
	# Show Endian Info
	# ============================================================================
	def show_endian
		endian_str = ""
		case @elf_endian
		when :ELF_LITTLE_ENDIAN
			endian_str = "2's complement, little endian"
		when :ELF_BIG_ENDIAN
			endian_str = "2's complement, big endian"
		else
			endian_str = "Invalid Endian"
		end
		puts "  Data:                              #{endian_str}"
	end

	# ============================================================================
	# Show ELF Version Info
	# ============================================================================
	def show_elf_version
		ver_str = "#{@elf_version}"
		ver_str += " (current)" if @elf_version == 1
		puts "  Version:                           #{ver_str}"
	end

	# ============================================================================
	# Show OS ABI Info
	# ============================================================================
	def show_OS_ABI
		abi_str = ""
		case @os_abi
		when :OS_ABI_UNIX
			abi_str = "UNIX - System V"
		when :OS_ABI_LINUX
			abi_str = "Linux"
		else
			abi_str = "undefined OS ABI"
		end
		puts  "  OS/ABI:                            #{abi_str}"
	end

	def show_ABI_version
		 puts "  ABI Version:                       #{@os_abi_version}"
	end

	# ============================================================================
	# Show ELF File Type Info
	# ============================================================================
	def show_file_type
		str = ""
		case @elf_type
		when ELF_ET_REL
			str = "REL (Relocatable file)"
	 	when ELF_ET_EXEC
			str = "EXEC (Executable file)"
		when ELF_ET_DYN
			str = "DYN (Shared object file)"
		when ELF_ET_CORE
			str = "CORE (Core file)"
		else
			str = "Invalid ELF Type #{@elf_type}"
		end
		puts "  Type:                              #{str}"
	end

	# ============================================================================
	# Show ELF Machine Archtecture Info
	# ============================================================================
	def show_machine_arch
		puts "  Machine:                           #{ELF_MACHINE_ARCH_LIST[@elf_machine.to_i]} (#{@elf_machine.to_i})"
	end

	# ============================================================================
	# Show Entry Point Address
	# ============================================================================
	def show_entry_point
		puts "  Entry point address:               #{@elf_entry.to_h}"
	end

	# ============================================================================
	# Show ELF File Version
	# ============================================================================
	def show_file_version
		puts "  Version:                           #{@elf_version.to_h}"
	end

	# ============================================================================
	# Show Program Header info
	# ============================================================================
	def show_program_h_offset
		puts "  Start of program headers:          #{@elf_program_h_offset.to_h} (bytes into file)"
	end

	# ============================================================================
	# Show Section Header info
	# ============================================================================
	def show_section_h_offset
		puts "  Start of section headers:          #{@elf_section_h_offset} (bytes into file)"
	end

	# ============================================================================
	# Show ELF Flags info (ELF Flags is currently not used)
	# ============================================================================
	def show_elf_flags
			puts "  Flags:                             #{@elf_flags.to_h}"
	end

	# ============================================================================
	# Show size of ELF header.
	# ============================================================================
	def show_elf_h_size
		puts "  Size of this header:               #{@elf_h_size} (bytes)"
	end

	# ============================================================================
	# Show number of section headers.
	# ============================================================================
	def show_program_h_size
		puts "  Size of program headers:           #{@elf_program_h_size} (bytes)"
	end

	# ============================================================================
	# Show number of program headers.
	# ============================================================================
	def show_program_h_num
		puts "  Number of program headers:         #{@elf_program_h_num}"
	end

	# ============================================================================
	# Show size of section headers.
	# ============================================================================
	def show_section_h_size
		puts "  Size of section headers:           #{@elf_section_h_size} (bytes)"
	end

	# ============================================================================
	# Show number of section headers.
	# ============================================================================
	def show_section_h_num
		puts "  Number of section headers:         #{@elf_section_h_num}"
	end

	# ============================================================================
	# Show section name string table index.
	# ============================================================================
	def show_section_name_idx
		 puts "  Section header string table index: #{@elf_section_name_idx}"
	end

	# ============================================================================
	# get section info from section binary data.
	# ============================================================================
	def get_section_info section_header
		section_info = {}
		if @elf_class == :CLASS_ELF32
			address_size = ELF_SIZE_ADDR_32
			offset_size  = ELF_SIZE_OFFSET_32
		else
			address_size = ELF_SIZE_ADDR_64
			offset_size  = ELF_SIZE_OFFSET_64
		end

		# index of name string in .shstrtab
		pos = 0
		section_info[:name_idx] = section_header[pos, ELF_SIZE_WORD].to_i
		pos += ELF_SIZE_WORD

		# section type
		section_info[:type] = section_header[pos, ELF_SIZE_WORD].to_i
		pos += ELF_SIZE_WORD

		# section flags
		section_info[:flags] = section_header[pos, ELF_SIZE_WORD].to_i
		pos += ELF_SIZE_WORD

		# address when section loaded to memory.
		section_info[:va_address] = section_header[pos, address_size].to_i
		pos += address_size

		# offset position of this secion in file.
		section_info[:offset] = section_header[pos, offset_size].to_i
		pos += offset_size

		# section size
		section_info[:size] = section_header[pos, ELF_SIZE_WORD].to_i
		pos += ELF_SIZE_WORD

		# Index of related section
		section_info[:link] = section_header[pos, ELF_SIZE_WORD].to_i
		pos += ELF_SIZE_WORD

		# Depends on section type.
		section_info[:info] = section_header[pos, ELF_SIZE_WORD].to_i
		pos += ELF_SIZE_WORD

		# Allignment size.
		section_info[:addr_align] = section_header[pos, ELF_SIZE_WORD].to_i
		pos += ELF_SIZE_WORD

		# Section entry size (used when section has struct table)
		section_info[:entry_size] =  section_header[pos, ELF_SIZE_WORD].to_i
		section_info
	end

	def initialize_section_h_map

		# =======================================================
		# Get '.shstrtab' section data for take each section name.
		# =======================================================
		pos = @elf_section_h_offset + (@elf_section_name_idx * @elf_section_h_size)
		names_section_header = @bin[pos, @elf_section_h_size]
		section_info = get_section_info names_section_header
		names_section_pos  = section_info[:offset]
		names_section_size = section_info[:size]
		names_section = @bin[names_section_pos, names_section_size]

		# =======================================================
		# Get section info by section headers.
		# =======================================================
		idx = 0
		while idx < @elf_section_h_num
			pos = @elf_section_h_offset + (idx * @elf_section_h_size)
			section_header = @bin[pos, @elf_section_h_size]
			section_info = get_section_info section_header
			name_pos = section_info[:name_idx]
			len = names_section.length - name_pos
			# get section name from '.shstrtab' section
			section_info[:name] = names_section[name_pos, len].c_str
			puts section_info
			idx += 1
		end

=begin
		@section_h_map = {}

		# calc section names section pos.
		pos = @elf_section_h_offset + (@elf_section_name_idx * @elf_section_h_size)
		puts pos.to_h
		puts @names_section.hex_dump
		section_idx = 0
		while section_idx < @elf_section_h_num
			section_pos = @elf_section_h_offset + (section_idx * @elf_section_h_size)
			section_data = @bin[section_pos, @elf_section_h_size]
			sec_idx += 1

			# .shstrtabにおけるセクション名のオフセット位置を取得
			name_offset = section[0, ELF_SIZE_WORD].to_i
			name = @names_section[name_offset, @names_sec_length].c_str

			# セクションヘッダのインデックスを設定
			@section_h_map[name] = section_idx
		end
=end
	end

	# ============================================================================
	# Check ELF Magic Number 0x7F ELF
	# ============================================================================
	def is_elf? elf_identifer
		return false if elf_identifer[0] != 0x7F
		return false if elf_identifer[1] != 'E'.ord
		return false if elf_identifer[2] != 'L'.ord
		return false if elf_identifer[3] != 'F'.ord
		true
	end


	def show_symtab_info
	end
end
