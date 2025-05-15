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

#ifndef AUDIOWMARK_SYNC_FINDER_HH
#define AUDIOWMARK_SYNC_FINDER_HH

#include "convcode.hh"
#include "wavdata.hh"
#include "random.hh"
#include "threadpool.hh"

/*
 * The SyncFinder class searches for sync bits in an input WavData. It is used
 * by both, the BlockDecoder and ClipDecoder to find a time index where
 * decoding should start.
 *
 * The first step for finding sync bits is search_approx, which generates a
 * list of approximate locations where sync bits match, using a stepping of
 * sync_search_step=256 (for a frame size of 1024). The approximate candidate
 * locations are later refined with search_refine using sync_search_fine=8 as
 * stepping.
 *
 * BlockDecoder and ClipDecoder have similar but not identical needs, so
 * both use this class, using either Mode::BLOCK or Mode::CLIP.
 *
 * BlockDecoder (Mode::BLOCK)
 *  - search for full A or full B blocks
 *  - select candidates by threshold(s) only
 *  - zero samples are not treated any special
 *
 * ClipDecoder (Mode::CLIP)
 *  - search for AB block (one A block followed by one B block) or BA block
 *  - select candidates by threshold, but only keep at most the 5 best matches
 *  - zero samples at beginning/end don't affect the score returned by sync_decode
 *  - zero samples at beginning/end don't cost much cpu time (no fft performed)
 *
 * The ClipDecoder will always use a big amount of zero padding at the beginning
 * and end to be able to find "partial" AB blocks, where most of the data is
 * matched with zeros.
 *
 * ORIG:   |AAAAA|BBBBB|AAAAA|BBBBB|
 * CLIP:                   |A|BB|
 * ZEROPAD:           00000|A|BB|00000
 * MATCH                AAAAA|BBBBB
 *
 * In this example a clip (CLIP) is generated from an original file (ORIG).  By
 * zero padding we get a file that contains the clip (ZEROPAD). Finally we are
 * able to match an AB block to the zeropadded file (MATCH). This gives us an
 * index in the zeropadded file that can be used for decoding the available
 * data.
 */
class SyncFinder
{
public:
  enum class Mode { BLOCK, CLIP };

  struct Score {
    size_t        index;
    double        quality;
    ConvBlockType block_type;
  };
  struct FrameBit
  {
    int frame;
    std::vector<int> up;
    std::vector<int> down;
  };
  struct KeyResult
  {
    Key                key;
    std::vector<Score> sync_scores;
  };
private:
  static constexpr int local_mean_distance = 20;
  struct SearchScore {
    size_t index;
    double raw_quality;
    double local_mean;

    double abs_quality() const
    {
      return fabs (raw_quality - local_mean);
    }
  };
  struct SearchKeyResult {
    Key                      key;
    std::vector<SearchScore> scores;
  };
  double  sync_decode (const std::vector<std::vector<FrameBit>>& sync_bits,
                       const size_t start_frame,
                       const std::vector<float>& fft_out_db,
                       const std::vector<char>&  have_frames);
  void scan_silence (const WavData& wav_data);
  void search_approx (std::vector<SearchKeyResult>& key_results, const std::vector<std::vector<std::vector<FrameBit>>>& sync_bits, const WavData& wav_data, Mode mode);
  void sync_select_local_maxima (std::vector<SearchScore>& sync_scores);
  void sync_mask_avg_false_positives (std::vector<SearchScore>& sync_scores);
  void sync_select_by_threshold (std::vector<SearchScore>& sync_scores);
  void sync_select_threshold_and_n_best (std::vector<SearchScore>& sync_scores, double threshold);
  void sync_select_truncate_n (std::vector<SearchScore>& sync_scores, size_t n);
  void search_refine (const WavData& wav_data, Mode mode, SearchKeyResult& key_result, const std::vector<std::vector<FrameBit>>& sync_bits);
  std::vector<KeyResult> fake_sync (const std::vector<Key>& key_list, const WavData& wav_data, Mode mode);

  // non-zero sample range: [wav_data_first, wav_data_last)
  size_t wav_data_first = 0;
  size_t wav_data_last = 0;
public:
  std::vector<KeyResult> search (const std::vector<Key>& key_list, const WavData& wav_data, Mode mode);
  static std::vector<std::vector<FrameBit>> get_sync_bits (const Key& key, Mode mode);

  static double bit_quality (float umag, float dmag, int bit);
  static double normalize_sync_quality (double raw_quality);
private:
  void sync_fft_parallel (ThreadPool& thread_pool,
                          const WavData& wav_data,
                          size_t index,
                          std::vector<float>& fft_out_db,
                          std::vector<char>& have_frames);
  void sync_fft (const WavData& wav_data,
                 size_t index,
                 size_t frame_count,
                 std::vector<float>& fft_out_db,
                 std::vector<char>& have_frames,
                 const std::vector<char>& want_frames);
  std::string find_closest_sync (size_t index);
  std::vector<std::vector<int>> split_vector (std::vector<int>& in_vector, size_t max_size);
};

#endif
