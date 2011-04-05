require 'speech'

class Dur < Speech::AudioInspector::Duration
end

accom = Dur.new("00:00:00.00")

merge_set = []
# decades
["00:00:00.32",
"00:00:04.73",
"00:00:01.82",
"00:00:05.37",
"00:00:05.08",
"00:00:00.60",
"00:00:05.21",
"00:00:01.28",
"00:00:01.34",
"00:00:02.17",
"00:00:00.32",
"00:00:03.71",
"00:00:01.44",
"00:00:03.61",
"00:00:00.41",
"00:00:05.34",
"00:00:01.12",
"00:00:00.99",
"00:00:02.75",
"00:00:01.98",
"00:00:01.28"].each do|stamp|
  d = Dur.new(stamp)
  puts stamp
  accom = accom + d
#  puts "#{accom.to_s} -> #{accom.to_f}"
#  puts "\tstore: #{stamp} -> #{merge_set.inspect} = #{accom}"
  merge_set << d

  if accom.to_f > 10.0
    last = merge_set.pop
    accom = Dur.new("00:00:00.00")
    merge_set.each {|s| accom = accom + s }
    puts "\tmerge(#{accom.to_s}): #{merge_set.inspect}"
    merge_set = [last]
    accom = Dur.from_seconds(last.to_f)
  end

end
