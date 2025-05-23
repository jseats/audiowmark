/*
 * Copyright (C) 2018-2020 Stefan Westerfeld
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdoutwavoutputstream.hh"
#include "utils.hh"

#include <assert.h>
#include <string.h>
#include <math.h>
#include <errno.h>

using std::string;
using std::vector;
using std::min;

StdoutWavOutputStream::~StdoutWavOutputStream()
{
  close();
}

int
StdoutWavOutputStream::sample_rate() const
{
  return m_sample_rate;
}

int
StdoutWavOutputStream::bit_depth() const
{
  return m_bit_depth;
}

int
StdoutWavOutputStream::n_channels() const
{
  return m_n_channels;
}

static void
header_append_str (vector<unsigned char>& bytes, const string& str)
{
  for (auto ch : str)
    bytes.push_back (ch);
}

static void
header_append_u32 (vector<unsigned char>& bytes, uint32_t u)
{
  bytes.push_back (u);
  bytes.push_back (u >> 8);
  bytes.push_back (u >> 16);
  bytes.push_back (u >> 24);
}

static void
header_append_u16 (vector<unsigned char>& bytes, uint16_t u)
{
  bytes.push_back (u);
  bytes.push_back (u >> 8);
}

Error
StdoutWavOutputStream::open (int n_channels, int sample_rate, int bit_depth, Encoding encoding, size_t n_frames, bool wav_pipe)
{
  assert (m_state == State::NEW);

  if (encoding == Encoding::FLOAT)
    {
      if (bit_depth != 32 && bit_depth != 64)
        {
          return Error (string_printf ("StdoutWavOutputStream::open: unsupported floating point bit depth %d", bit_depth));
        }
    }
  else if (bit_depth != 16 && bit_depth != 24 && bit_depth != 32)
    {
      return Error (string_printf ("StdoutWavOutputStream::open: unsupported bit depth %d", bit_depth));
    }
  if (n_frames == AudioInputStream::N_FRAMES_UNKNOWN && !wav_pipe)
    {
      return Error ("unable to write wav format to standard out without input length information");
    }

  RawFormat format;
  format.set_bit_depth (bit_depth);
  format.set_encoding (encoding);

  Error err = Error::Code::NONE;
  m_raw_converter.reset (RawConverter::create (format, err));
  if (err)
    return err;

  vector<unsigned char> header_bytes;

  size_t data_size = n_frames * n_channels * ((bit_depth + 7) / 8);

  m_close_padding = data_size & 1; // padding to ensure even data size
  size_t aligned_data_size = data_size + m_close_padding;

  header_append_str (header_bytes, "RIFF");
  if (wav_pipe)
    header_append_u32 (header_bytes, -1);
  else
    header_append_u32 (header_bytes, 36 + aligned_data_size);
  header_append_str (header_bytes, "WAVE");

  // subchunk 1
  header_append_str (header_bytes, "fmt ");
  header_append_u32 (header_bytes, 16); // subchunk size
  header_append_u16 (header_bytes, encoding == Encoding::FLOAT ? 3 : 1);  // uncompressed audio
  header_append_u16 (header_bytes, n_channels);
  header_append_u32 (header_bytes, sample_rate);
  header_append_u32 (header_bytes, sample_rate * n_channels * bit_depth / 8); // byte rate
  header_append_u16 (header_bytes, n_channels * bit_depth / 8); // block align
  header_append_u16 (header_bytes, bit_depth); // bits per sample

  // subchunk 2
  header_append_str (header_bytes, "data");
  if (wav_pipe)
    header_append_u32 (header_bytes, -1);
  else
    header_append_u32 (header_bytes, data_size);

  fwrite (&header_bytes[0], 1, header_bytes.size(), stdout);
  if (ferror (stdout))
    return Error ("write wav header failed");

  m_bit_depth   = bit_depth;
  m_sample_rate = sample_rate;
  m_n_channels  = n_channels;
  m_state       = State::OPEN;

  return Error::Code::NONE;
}

Error
StdoutWavOutputStream::write_frames (const vector<float>& samples)
{
  if (samples.empty())
    return Error::Code::NONE;

  const size_t block_size = 8192 * m_n_channels;
  const int sample_width = m_bit_depth / 8;

  m_output_bytes.resize (sample_width * block_size);
  size_t pos = 0;

  while (size_t todo = min (block_size, samples.size() - pos))
    {
      m_raw_converter->to_raw (samples.data() + pos, m_output_bytes.data(), todo);

      fwrite (m_output_bytes.data(), 1, todo * sample_width, stdout);
      if (ferror (stdout))
        return Error (string_printf ("write sample data failed (%s)", strerror (errno)));

      pos += todo;
    }
  return Error::Code::NONE;
}

Error
StdoutWavOutputStream::close()
{
  if (m_state == State::OPEN)
    {
      for (size_t i = 0; i < m_close_padding; i++)
        {
          fputc (0, stdout);
          if (ferror (stdout))
            return Error ("write wav padding failed");
        }
      fflush (stdout);
      if (ferror (stdout))
        return Error ("error during flush");

      m_state = State::CLOSED;
    }
  return Error::Code::NONE;
}
