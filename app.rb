#!/usr/bin/ruby
$LOAD_PATH.push("./ELF")
require "elf"
require "elf_object"
require "rx_linker"

if ARGV.length < 1
  puts "input object file."
  exit 1
end

elf_ojects = []

elf_class = nil
ARGV.each do |filepath|
  elf_object = ELF::ElfObject.new(filepath)
  if elf_class.nil?
    elf_class = elf_object.elf_class
  else
    throw "Different Elf Class found!" if elf_class != elf_object.elf_class
  end

  #elf_object.section_h_map.delete_if { |key, val| val[:size] == 0 }
  elf_ojects << elf_object
end

# Link Object files.
linker = ELF::RXLinker.new
linker.link "sakura2.abs", elf_ojects, "test/led/sakura2.clnk"
