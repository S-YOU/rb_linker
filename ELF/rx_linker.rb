require 'elf'
require 'linker'
module ELF

	RX_SECTIONS = [
		"P",									# �v���O�����̈�				1byte
		"C",									# const �f�[�^�̈�			4byte
		"C_2",								# const �f�[�^�̈�			2byte
		"C_1",								# const �f�[�^�̈�			2byte
		"D",									# �����l����f�[�^			4byte
		"D_2",								# �����l����f�[�^			2byte
		"D_1",								# �����l����f�[�^			1byte
		"SU",									# ���[�U�[�X�^�b�N�̈�	4byte
		"SI",									# ���荞�݃X�^�b�N			4byte
		"C$VECT",							# �σx�N�^�e�[�u��(���荞�݃n���h��)
		"C$DSEC",							# �����l����f�[�^�Z�N�V�����e�[�u��
		"C$BSEC",							# �����l�Ȃ��f�[�^�Z�N�V�����p�e�[�u��
		"FIXEDVECT",					# �Œ�x�N�^�e�[�u��(���荞�݃n���h��)
		"PResetPRG",					# ���Z�b�g�x�N�^
		".symtab",						# �V���{���e�[�u��
		".debug_info",				# �f�o�b�O���
		".debug_abbrev",			# ?
		".debug_line",				# ?
		".debug_pubnames",		# ?
		".debug_aranges",			# ?
		".debug_frame",				# ?
		".debug_loc",					# ?
		".strtab",						# ������e�[�u��
		".shstrtab"						# �Z�N�V����������
	]

	class RXLinker < Linker

	  def check_elf_header elf_objects
	    # check ELF Header of each objects
	    true
	  end

	  def link filepath, elf_objects
	    check_elf_header(elf_objects)

	    elf_first = elf_objects.first
	    elf_objects = elf_objects
	    link_f = open(filepath, "wb")
			section_size = 0

			section_map = {}
	    elf_objects.each do |elf_object|
	    	RX_SECTIONS.each do |section_name|
	    		sec = elf_object.get_section_data(section_name)
	    		next if sec.nil?
	    		if section_map.has_key?(section_name)
	    			section_map[section_name].concat(sec)
	    		else
		    		section_map[section_name] = sec
	    		end
	    	end
	    end

			# ELF header
	    out_elf_header(link_f, elf_first, elf_objects)

			# write secions
			RX_SECTIONS.each do |section_name|
				sec = section_map[section_name]
				link_f.write(sec.pack("C*"))	unless sec.nil?
			end

	  end
	end
end
