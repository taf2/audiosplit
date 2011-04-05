#!/usr/bin/env ruby
# see: http://chillibear.org/2010/04/comparing-version-numbers-with-ruby.html
# Revision class
# Designed to function in version comparisons, where the
# version number matches /\d+(\.\d+)*/

# Author: Hugh Sasse, De Montfort University <hgs@dmu.ac.uk>

# Created: 06-OCT-2000
# Last Modified: 13-APR-2010

class Revision

    attr :contents

    def to_i
      @contents.collect { |j| j.to_i }
    end

    def to_s
      astring = @contents.collect { |j| j.to_s }
      astring.join(".")
    end

    def <=>(x)
       @contents <=> x.contents
    end

    def <(x)
       comparison = @contents <=> x.contents
       case comparison
           when -1
               result = true
           when 0
               result = false
           when 1
               result = false
       end
       result
    end

    def >(x)
       comparison = @contents <=> x.contents
       case comparison
           when -1
               result = false
           when 0
               result = false
           when 1
               result = true
       end
       result
    end

    def ==(x)
       comparison = @contents <=> x.contents
       case comparison
           when -1
               result = false
           when 0
               result = true
           when 1
               result = false
       end
       result
    end

    def initialize(thing)
        if thing.class == Revision
    @contents = thing.contents.dup
    # dup so we get a new copy, not a pointer to it.
  elsif thing.class == String
    result = thing.split(".").collect {|j| j.to_i}
    @contents = result
  elsif thing.class == Float
    @contents = thing.to_s.split(".").collect {|j| j.to_i}
  else
                raise "cannot initialise to #{thing}"
        end
    end

end

if __FILE__ == $0

    # Perform RubyUnit tests

    require "runit/testcase"
    require 'runit/cui/testrunner'
    require 'runit/testsuite'

    class  Testing_class < RUNIT::TestCase
        def test_a
            a = Revision.new("1.2.3")
            assert_equal([1,2,3], a.contents, "initialize failed #{a.contents}")
            a = Revision.new("2.4.8")
            b = Revision.new(a)
            assert_equal([2,4,8], b.contents, "initialize failed #{a.contents}")
            a = Revision.new(2.3)
            assert_equal([2,3], a.contents, "initialize failed #{a.contents}")
        end
        def test_equal
            a = Revision.new("1.2.1")
            b = Revision.new("1.2.1")
            assert(a == b, "supposedly identical Revisions are not.")
            a = Revision.new("2.2.2")
            b = Revision.new("2.2.1")
            assert(!(a == b), "supposedly inidentical Revisions are not.")
        end
        def test_less_than
            a = Revision.new("1.2.1")
            b = Revision.new("1.2.2")
            assert(a < b, " 1.2.1 not less that 1.2.2 ")
            a = Revision.new("1.2.2")
            b = Revision.new("1.3.2")
            assert(a < b, " 1.2.2 not less that 1.3.2 ")
            a = Revision.new("1.2.2")
            b = Revision.new("2.2.2")
            assert(a < b, " 1.2.2 not less that 2.2.2 ")
            a = Revision.new("1.2.3.4")
            b = Revision.new("1.2.3.5")
            assert(a < b, " 1.2.3.4 not less that 1.2.3.5 ")
            a = Revision.new("1.2.3")
            b = Revision.new("1.2.3.5")
            assert(a < b, " 1.2.3 not less that 1.2.3.5 ")
            # check comparison is numeric, not lexical
            a = Revision.new("1.2.3")
            b = Revision.new("1.12.3")
            assert(a < b, " 1.2.3 not less that 1.2.3.5 ")
        end
        def test_greater_than
            a = Revision.new("1.2.2")
            b = Revision.new("1.2.1")
            assert(a > b, " 1.2.1 not less that 1.2.2 ")
            a = Revision.new("1.3.2")
            b = Revision.new("1.2.2")
            assert(a > b, " 1.2.2 not less that 1.3.2 ")
            a = Revision.new("2.2.2")
            b = Revision.new("1.2.2")
            assert(a > b, " 1.2.2 not less that 2.2.2 ")
            a = Revision.new("1.2.3.5")
            b = Revision.new("1.2.3.4")
            assert(a > b, " 1.2.3.4 not less that 1.2.3.5 ")
            a = Revision.new("1.2.3.5")
            b = Revision.new("1.2.3")
            assert(a > b, " 1.2.3 not less that 1.2.3.5 ")
            # check comparison is numeric, not lexical
            a = Revision.new("1.12.3")
            b = Revision.new("1.2.3")
            assert(a > b, " 1.2.3 not less that 1.2.3.5 ")
        end
    end

    RUNIT::CUI::TestRunner.run(Testing_class.suite)
end
