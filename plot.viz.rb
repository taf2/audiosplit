require 'rubygems'
require 'gruff'
# given an input stream of wave lengths plot into a wave graph... NOTE: this is not optimized for large wave files

g = Gruff::Line.new
g.data "Wave", STDIN.read.split(' ').map {|i| i.to_i }

g.write 'out.wave.png'
