require 'rubygems'
require 'gruff'

g = Gruff::Line.new
g.data "Wave", STDIN.read.split(' ').map {|i| i.to_i }

g.write 'out.wave.png'
