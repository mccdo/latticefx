#!/usr/bin/env ruby
require 'optparse'
require 'ostruct'

class ArgvOptionsParser
   def self.parse(args)
      options = OpenStruct.new
      options.classname = []
      options.filenames = []
      options.namespace = ""
      options.path = ""
      options.add_copyright = true

      opts = OptionParser.new do |opts|
         opts.banner = "Usage: FileUtils.rb [options]"
         opts.separator ""
         
         opts.on("-u", "--update file(s)",
              "File to update copyright for") do |filename|
                    options.filenames << filename
         end

         opts.on_tail("-h", "--help", "Show this message") do
            puts opts
            exit
         end
      end

      opts.parse!
      options
   end
end

options = ArgvOptionsParser.parse(ARGV)
copyright = []
copyright << "/*************** <auto-copyright.rb BEGIN do not edit this line> **************\n"
copyright << " *\n"
copyright << " * Copyright 2012-2012 by Ames Laboratory\n"
copyright << " *\n"
copyright << " * This library is free software; you can redistribute it and/or\n"
copyright << " * modify it under the terms of the GNU Library General Public\n"
copyright << " * License version 2.1 as published by the Free Software Foundation.\n"
copyright << " *\n"
copyright << " * This library is distributed in the hope that it will be useful,\n"
copyright << " * but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
copyright << " * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
copyright << " * Library General Public License for more details.\n"
copyright << " *\n"
copyright << " * You should have received a copy of the GNU Library General Public\n"
copyright << " * License along with this library; if not, write to the\n"
copyright << " * Free Software Foundation, Inc., 59 Temple Place - Suite 330,\n"
copyright << " * Boston, MA 02111-1307, USA.\n"
copyright << " *\n"
copyright << " *************** <auto-copyright.rb END do not edit this line> ***************/\n"

options.filenames.each do |file_name|
  if File.file? file_name
    file = File.new(file_name)
    lines = file.readlines
    file.close

    changes = false
    lines.each do |line|
      changes = true if line.include? "<auto-copyright.rb BEGIN"
    end
    
    if not changes
      no_header = ""
      no_header += file_name.to_s
      no_header += " does not contain a copyright header"
      puts no_header
    end

    # Don't write the file if there are no changes
    copyright_changing = false
    if changes
      file = File.new(file_name,'w')
      lines.each do |line|
        if line.include? "<auto-copyright.rb BEGIN"
          copyright_changing = true
          file_changing = "Changing: "
          file_changing += file_name.to_s
          puts file_changing
          copyright.each do |cp_line|
            file.write(cp_line)
          end
        end
        if not copyright_changing
          file.write(line)
        end
        copyright_changing = false if line.include? "<auto-copyright.rb END"
      end
      file.close
    end
  end
end

