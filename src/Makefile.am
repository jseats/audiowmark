dist_bin_SCRIPTS = videowmark

COMMON_SRC = utils.hh utils.cc convcode.hh convcode.cc random.hh random.cc wavdata.cc wavdata.hh \
	     audiostream.cc audiostream.hh sfinputstream.cc sfinputstream.hh stdoutwavoutputstream.cc stdoutwavoutputstream.hh \
	     sfoutputstream.cc sfoutputstream.hh rawinputstream.cc rawinputstream.hh rawoutputstream.cc rawoutputstream.hh \
	     rawconverter.cc rawconverter.hh mp3inputstream.cc mp3inputstream.hh wmcommon.cc wmcommon.hh fft.cc fft.hh \
	     limiter.cc limiter.hh shortcode.cc shortcode.hh mpegts.cc mpegts.hh hls.cc hls.hh audiobuffer.hh \
	     wmget.cc wmadd.cc syncfinder.cc syncfinder.hh wmspeed.cc wmspeed.hh threadpool.cc threadpool.hh \
	     resample.cc resample.hh wavpipeinputstream.cc wavpipeinputstream.hh wavchunkloader.cc wavchunkloader.hh
COMMON_LIBS = $(SNDFILE_LIBS) $(FFTW_LIBS) $(LIBGCRYPT_LIBS) $(LIBMPG123_LIBS) $(FFMPEG_LIBS) $(ZITA_LIBS)

AM_CXXFLAGS = $(SNDFILE_CFLAGS) $(FFTW_CFLAGS) $(LIBGCRYPT_CFLAGS) $(LIBMPG123_CFLAGS) $(FFMPEG_CFLAGS) $(ZITA_CFLAGS)



noinst_PROGRAMS = testconvcode testrandom testmp3 teststream testlimiter testshortcode testmpegts testthreadpool \
		  testrawconverter testwavformat

TEST_LDADD = libaudiowmark.la $(COMMON_LIBS)

testconvcode_SOURCES = testconvcode.cc 
testconvcode_LDADD = $(TEST_LDADD)

testrandom_SOURCES = testrandom.cc 
testrandom_LDADD = $(TEST_LDADD)

testmp3_SOURCES = testmp3.cc 
testmp3_LDADD = $(TEST_LDADD)

teststream_SOURCES = teststream.cc 
teststream_LDADD = $(TEST_LDADD)

testlimiter_SOURCES = testlimiter.cc 
testlimiter_LDADD = $(TEST_LDADD)

testshortcode_SOURCES = testshortcode.cc 
testshortcode_LDADD = $(TEST_LDADD)

testmpegts_SOURCES = testmpegts.cc 
testmpegts_LDADD = $(TEST_LDADD)

testthreadpool_SOURCES = testthreadpool.cc 
testthreadpool_LDADD = $(TEST_LDADD)

testrawconverter_SOURCES = testrawconverter.cc 
testrawconverter_LDADD = $(TEST_LDADD)

testwavformat_SOURCES = testwavformat.cc 
testwavformat_LDADD = $(TEST_LDADD)

if COND_WITH_FFMPEG
COMMON_SRC += hlsoutputstream.cc hlsoutputstream.hh

noinst_PROGRAMS += testhls
testhls_SOURCES = testhls.cc 
testhls_LDFLAGS = $(COMMON_LIBS)
endif

# libaudiowmark
lib_LTLIBRARIES = libaudiowmark.la

libaudiowmark_la_SOURCES = $(COMMON_SRC)
libaudiowmark_la_LDFLAGS = -version-info 0:0:0
libaudiowmark_la_LIBADD  = $(COMMON_LIBS)

# ---------- CLI program (now links the library) ----------
bin_PROGRAMS = audiowmark

# 
audiowmark_SOURCES = audiowmark.cc
audiowmark_LDADD   = libaudiowmark.la 
