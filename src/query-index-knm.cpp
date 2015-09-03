#include <sdsl/int_vector.hpp>
#include <sdsl/int_vector_mapper.hpp>
#include "sdsl/suffix_arrays.hpp"
#include "sdsl/suffix_trees.hpp"
#include <sdsl/suffix_array_algorithm.hpp>
#include <iostream>
#include <math.h>
#include <algorithm>
#include <string>
#include <iomanip>

#include "utils.hpp"
#include "collection.hpp"
#include "index_types.hpp"

#include "knm.hpp"

#include "logging.hpp"

using namespace std::chrono;

typedef struct cmdargs {
    std::string pattern_file;
    std::string collection_dir;
    int ngramsize;
    bool ismkn;
    bool isbackward;
    bool byte_alphabet;
} cmdargs_t;

std::vector<uint32_t> ngram_occurrences;

void print_usage(const char* program)
{
    fprintf(stdout, "%s -c <collection dir> -p <pattern file> -m <boolean> -n <ngramsize>\n",
            program);
    fprintf(stdout, "where\n");
    fprintf(stdout, "  -c <collection dir>  : the collection dir.\n");
    fprintf(stdout, "  -p <pattern file>  : the pattern file.\n");
    fprintf(stdout, "  -m : use Modified-KN (default = KN).\n");
    fprintf(stdout, "  -n <ngramsize>  : the ngramsize (integer).\n");
    fprintf(stdout, "  -b : use faster index lookup using only backward search (default = forward+backward).\n");
    fprintf(stdout, "  -1 : byte parsing.\n");
};

cmdargs_t parse_args(int argc, const char* argv[])
{
    cmdargs_t args;
    int op;
    args.pattern_file = "";
    args.collection_dir = "";
    args.ismkn = false;
    args.ngramsize = 1;
    args.isbackward = false;
    args.byte_alphabet = false;
    while ((op = getopt(argc, (char* const*)argv, "p:c:n:mb1")) != -1) {
        switch (op) {
        case 'p':
            args.pattern_file = optarg;
            break;
        case 'c':
            args.collection_dir = optarg;
            break;
        case 'm':
            args.ismkn = true;
            break;
        case 'n':
            args.ngramsize = atoi(optarg);
            break;
        case 'b':
            args.isbackward = true;
            break;
        case '1':
            args.byte_alphabet = true;
            break;
        }
    }
    if (args.collection_dir == "" || args.pattern_file == "") {
        LOG(ERROR) << "Missing command line parameters.\n";
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    return args;
}

// fast_index = true -- use N1+Back/FrontBack based solely on forward CST & backward search
//            = false -- use N1+Back/FrontBack using reverse CST & forward search
template <class t_idx>
void run_queries(const t_idx& idx, const std::vector<std::vector<uint64_t> > patterns,
                 uint64_t ngramsize, bool fast_index, bool ismkn)
{
    using clock = std::chrono::high_resolution_clock;
    double perplexity = 0;
    uint64_t M = 0;
    std::chrono::nanoseconds total_time(0);
    //uint64_t ind = 1;
    lm_bench::reset();
    for (std::vector<uint64_t> pattern : patterns) {
        uint64_t pattern_size = pattern.size();
        std::string pattern_string;
        M += pattern_size + 1; // +1 for adding </s>
        //        if(pattern.back() ==UNKNOWN_SYM) M--;
        pattern.push_back(PAT_END_SYM);
        pattern.insert(pattern.begin(), PAT_START_SYM);
        // run the query
        auto start = clock::now();
        double sentenceprob = sentence_logprob_kneser_ney(idx, pattern, M, ngramsize, fast_index, ismkn);
        auto stop = clock::now();

        //std::ostringstream sp("", std::ios_base::ate);
        //std::copy(pattern.begin(),pattern.end(),std::ostream_iterator<uint64_t>(sp," "));
        //LOG(INFO) << "P(" << ind++ << ") = " << sp.str() << "("<<
        //duration_cast<microseconds>(stop-start).count() / 1000.0f <<" ms)";

        perplexity += sentenceprob;
        total_time += (stop - start);
    }
    lm_bench::print();
    LOG(INFO) << "Time = " << duration_cast<microseconds>(total_time).count() / 1000.0f << " ms";
    perplexity = perplexity / M;
    LOG(INFO) << "Test Corpus Perplexity is: " << std::setprecision(10) << pow(10, -perplexity);
}

std::vector<std::string>
parse_line(const std::string& line, bool byte)
{
    std::vector<std::string> line_tokens;
    if (byte) {
        for (const auto& chr : line) {
            line_tokens.push_back(std::string(1, chr));
        }
    } else {
        std::istringstream input(line);
        std::string word;
        while (std::getline(input, word, ' ')) {
            line_tokens.push_back(word);
        }
    }
    return line_tokens;
}

template <class t_idx>
int execute(const cmdargs_t& args)
{
    /* load index */
    t_idx idx;
    auto index_file = args.collection_dir + "/index/index-" + sdsl::util::class_to_hash(idx) + ".sdsl";
    if (utils::file_exists(index_file)) {
        LOG(INFO) << "loading index from file '" << index_file << "'";
        sdsl::load_from_file(idx, index_file);
    } else {
        LOG(FATAL) << "index does not exist. build it first";
        return EXIT_FAILURE;
    }

    /* print precomputed parameters */
    idx.print_params(args.ismkn, args.ngramsize);

    /* load patterns */
    std::vector<std::vector<uint64_t> > patterns;
    std::vector<std::vector<std::string> > orig_patterns;
    if (utils::file_exists(args.pattern_file)) {
        std::ifstream ifile(args.pattern_file);
        LOG(INFO) << "reading input file '" << args.pattern_file << "'";
        std::string line;
        while (std::getline(ifile, line)) {
            auto line_tokens = parse_line(line, args.byte_alphabet);
            std::vector<uint64_t> tokens;
            std::vector<std::string> orig_tokens;
            for (const auto& word : line_tokens) {
                patterns.push_back(tokens);
                //LOG(INFO) << "\tpattern: " << idx.m_vocab.id2token(tokens.begin(), tokens.end());
            }
        }
    } else {
        LOG(FATAL) << "cannot read pattern file '" << args.pattern_file << "'";
        return EXIT_FAILURE;
    }

    /* runs the querying */
    run_queries(idx, patterns, args.ngramsize, args.isbackward, args.ismkn);

    return EXIT_SUCCESS;
}

int main(int argc, const char* argv[])
{
    log::start_log(argc, argv);

    /* parse command line */
    cmdargs_t args = parse_args(argc, argv);

    typedef index_succinct_compute_n1fb<default_cst_type, default_cst_rev_type> t_idx_compute;
    return execute<t_idx_compute>(args);
}
