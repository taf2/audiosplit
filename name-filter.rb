# convert chunk file names from name.wav.chunk\d to name.chunk\d.wav
Dir["#{ARGV[0]}*.wav.chunk*"].each do|wave|
  puts "#{wave} -> #{wave.gsub(/\.wav/,'')}.wav"
  system("mv #{wave} #{wave.gsub(/\.wav/,'')}.wav")
end
