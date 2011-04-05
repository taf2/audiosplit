# Silence Splitter

The idea of this is to make it easy to divide streams of audio up by quiet or silence within the stream.
The use of this library would be in making it possible to detect or split audio by word breaks in spoken language.

The library uses libsndfile 


algorithm...

goal - extract sequences of audio no longer than 10 seconds in length... attempt to locate word breaks at each 10 second interval...

read audio file in chunks of buffer size bytes, compute the root means square on all sampled frames within each sequence

if the audio chunk is below the threshold of min duration discard it

if the audio chunk is longer than the max duration than increase the threshold by 10 and split the new chunk with a byte buffer of half the original buffer
