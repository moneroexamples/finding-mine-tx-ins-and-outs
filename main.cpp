#include <iostream>
#include <string>

#include "src/MicroCore.h"
#include "src/CmdLineOptions.h"
#include "src/tools.h"

#include "mnemonics/electrum-words.h"

using namespace std;
using boost::filesystem::path;
using boost::filesystem::is_directory;

using xmreg::operator<<;

// without this it wont work. I'm not sure what it does.
// it has something to do with locking the blockchain and tx pool
// during certain operations to avoid deadlocks.
unsigned int epee::g_test_dbg_lock_sleep = 0;


int main(int ac, const char* av[]) {

    // get command line options
    xmreg::CmdLineOptions opts {ac, av};

    auto help_opt = opts.get_option<bool>("help");

    // if help was chosen, display help text and finish
    if (*help_opt)
    {
        return 0;
    }

    // get other options
    auto bc_path_opt  = opts.get_option<string>("bc-path");


    // default path to monero folder
    // on linux this is /home/<username>/.bitmonero
    string default_monero_dir = tools::get_default_data_dir();

    // the default folder of the lmdb blockchain database
    // is therefore as follows
    string default_lmdb_dir   = default_monero_dir + "/lmdb";

    // get the program command line options, or
    // some default values for quick check
    string viewkey_str  = "9c2edec7636da3fbb343931d6c3d6e11bcd8042ff7e11de98a8d364f31976c04";
    string spendkey_str = "950b90079b0f530c11801ef29e99618d3768d79d3d24972ff4b6fd9687b7b20c";
    path blockchain_path = bc_path_opt ? path(*bc_path_opt) : path(default_lmdb_dir);


    if (!is_directory(blockchain_path))
    {
        cerr << "Given path \"" << blockchain_path   << "\" "
             << "is not a folder or does not exist" << " "
             << endl;
        return 1;
    }

    blockchain_path = xmreg::remove_trailing_path_separator(blockchain_path);

    cout << "Blockchain path: " << blockchain_path << endl;

    // enable basic monero log output
    uint32_t log_level = 0;
    epee::log_space::get_set_log_detalisation_level(true, log_level);
    epee::log_space::log_singletone::add_logger(LOGGER_CONSOLE, NULL, NULL);


    // create instance of our MicroCore
    xmreg::MicroCore mcore;

    // initialize the core using the blockchain path
    if (!mcore.init(blockchain_path.string()))
    {
        cerr << "Error accessing blockchain." << endl;
        return 1;
    }



    // parse string representing given private viewkey
    crypto::secret_key private_view_key;

    if (!xmreg::parse_str_secret_key(viewkey_str, private_view_key))
    {
        cerr << "Cant parse view key: " << viewkey_str << endl;
        return 1;
    }

    // parse string representing given private viewkey
    crypto::secret_key private_spend_key;

    if (!xmreg::parse_str_secret_key(spendkey_str, private_spend_key))
    {
        cerr << "Cant parse spend key: " << spendkey_str << endl;
        return 1;
    }


    // we have private_spend_key, so now
    // we need to get the corresponding
    // public_spend_key
    crypto::public_key public_spend_key;


    // generate public key based on the private key
    crypto::secret_key_to_public_key(private_spend_key, public_spend_key);


    // we have private_spend_key, so now
    // we need to get the corresponding
    crypto::public_key public_view_key;


    // generate public key based on the private key
    crypto::secret_key_to_public_key(private_view_key, public_view_key);



    // parse string representing given monero address
    cryptonote::account_public_address address {public_spend_key, public_view_key};


    string mnemonic_str;
    string language {"English"};
    // derive the mnemonic version of the spend key.
    // 25 word mnemonic that is provided by the simplewallet
    // is just a word representation of the private spend key
    if (!crypto::ElectrumWords::bytes_to_words(private_spend_key, mnemonic_str, language))
    {
        cerr << "\nCant create the mnemonic for the private spend key: "
             << private_spend_key << endl;
        return 1;
    }



    cout << "\n"
         << "Private spend key: " << private_spend_key << "\n"
         << "Public spend key : " << public_spend_key  << endl;

    cout << "\n"
        << "Private view key : "  << private_view_key << "\n"
        << "Public view key  : "  << public_view_key  << endl;


    cout << "\n"
         << "Monero address   : "  << address << endl;

    cout << "\n"
         << "Mnemonic seed    : "  << mnemonic_str << endl;


    // for this example, I made new wallet. These are the transaction hashes
    // that send to the wallet, or send from the wallet. I hardcoded them here,
    // because its was much easier to work on the example. For more general
    // use, one would have to scan the blockchain to determine which
    // transactions our ours.
    vector<string> tx_hashes_str {
            "ead7b392f57311fbac14477c4a50bee935f1dbc06bf166d219f4c011ae1dc398",
            "50a3ded2df473a7e8a7fde58c8a865d1ae246ce8ceddb5f474164888fe2ad822",
            "fed715d3361f3c66437e1e19f193c93abb86c74b5d77ea464e95f27c37097214",
            "948a7ce9971d05e99a43f35e11e4c6a346e7d2b71758bed5cb4f9fc175f7bb5f",
            "edc9671f6f988f7d9a5265c1f7829d5b8ecc2f6ddafa995b23a1dd04f7834713",
            "7132cd214d9b7b502b2990b61181ba1cde203b9e4c648d60a9772e56c1ff2980",
            "60465baab286ae6656378ab5b036b32c238347e7d83988be6d613c17dc586cc4",
            "5dcc5eb9cd89f8d364f7ea4ad789067f8c16425ce94d6a6e38e1c0c3f1fedae6",
            "6f6d97eaa2de50d27b60ce8ac40b0b8dd53a56f7d9f17d81a71b29194d53dd58",
            "97ffac124e8215986cfa9f46fb4824d5f366e48cb5e30495a3debb62bac01c06",
            "6b524290bd6a955e72ad47d88736a871f7c2661f225e1127f4310c00e585f974",
            "a16f4658e736801ce1cc875e146127628810609b6297e362b86cd3c691d1a4d0",
            "1826bf767546ed1ebaebc22c34ca4f73e6f8b38efcb2ded79c9470d2625eadf9",
            "f81dd26e16c66a20e5609e6b19a849be7aaad3705ac7614db229a9d4f982bdaa",
            "87f64738a14d25a8e4e1a6c2a78510895b4dc6ea1ab4f909ff1ccde8c6907f10",
            "38b6935a9bc0385f5ddaf63582bb318a81125756a433fe2babab698f91438d20",
            "09d9e8eccf82b3d6811ed7005102caf1b605f325cf60ed372abeb4a67d956fff",
            "83d682b3f1b57db488b1d2040a48c0db957e8b79f5e6142e1b018f41d4d9dc84"
    };


    // get the highlevel cryptonote::Blockchain object to interact
    // with the blockchain lmdb database
    cryptonote::Blockchain& core_storage = mcore.get_core();

    vector<cryptonote::transaction> txs;

    // populate transactions vector
    for (const string& tx_hash_str: tx_hashes_str)
    {
        cryptonote::transaction tx;

        if (!xmreg::get_tx_from_str_hash(core_storage, tx_hash_str, tx))
        {
            cerr << "Cant find transaction with hash: " << tx_hash_str << endl;
            return 1;
        }

        txs.push_back(tx);
    }

    // store key images generated using our outputs
    // and our private spend key
    vector<crypto::key_image> key_images;

    // total xmr balance
    uint64_t total_xmr_balance {0};

    size_t tx_index {0};


    // for each transaction go through its outputs and inputs.
    // for outputs, check if any of the them belongs to us, based
    // on our private view key. If so, then get the xmr amount
    // sent to us, and also generate key image for each of our outputs.
    // key images are generated using our public spend key. each generated
    // key image we store in vector of the key_images.
    //
    // after we are done with outputs, we go to check inputs. inputs
    // are our spendings, but which input is ours? for this, we need
    // to check if input's key_image, matches any of ours key_images.
    // if there is a match, it means that this input is ours, i.e.,
    // we sent xmr somewhere.
    for (const cryptonote::transaction& tx: txs)
    {

        cout << "\n\n"
             << "********************************************************************\n"
             << "Transaction: "<< ++tx_index <<"\n"
             << "********************************************************************"
             << endl;

        // get tx public key from extras field
        crypto::public_key pub_tx_key = cryptonote::get_tx_pub_key_from_extra(tx);

        if (pub_tx_key == cryptonote::null_pkey)
        {
            cerr << "Cant get public key of tx with hash: "
                 << cryptonote::get_transaction_hash(tx)
                 << endl;

            return 1;
        }


        // public transaction key is combined with our private viewkey
        // to create, so called, derived key.
        crypto::key_derivation derivation;

        if (!generate_key_derivation(pub_tx_key, private_view_key, derivation))
        {
            cerr << "Cant get dervied key for: " << "\n"
                 << "pub_tx_key: " << private_view_key << " and "
                 << "private_view_key" << private_view_key << endl;

            return 1;
        }


        // lets check our keys
        cout << "\n"
             << "tx hash          : " << cryptonote::get_transaction_hash(tx) << "\n"
             << "public tx key    : "  << pub_tx_key << "\n"
             << "dervied key      : "  << derivation << "\n"
             << endl;


        //
        // check outputs to for incoming xmr
        //

        // get the total number of outputs in a transaction.
        size_t output_no = tx.vout.size();

        // sum amount of xmr sent to us
        // in the given transaction
        uint64_t money_received {0};


        // loop through outputs in the given tx
        // to check which outputs our ours. we compare outputs'
        // public keys with the public key that would had been
        // generated for us if we had gotten the outputs.
        for (size_t i = 0; i < output_no; ++i)
        {
            // get the tx output public key
            // that normally would be generated for us,
            // if someone had sent us some xmr.
            crypto::public_key pubkey;

            crypto::derive_public_key(derivation, i,
                                      public_spend_key,
                                      pubkey);

            // get tx output public key
            cryptonote::txout_to_key tx_out_to_key
                    = boost::get<cryptonote::txout_to_key>(tx.vout[i].target);


            cout << "Output no: " << i << ", " << tx_out_to_key.key;

            // check if the output's public key is ours
            if (tx_out_to_key.key == pubkey)
            {
                //
                // generate key_image of this output
                crypto::key_image key_image;

                if (!xmreg::generate_key_image(derivation, i,
                                               private_spend_key,
                                               public_spend_key,
                                               key_image))
                {
                    cerr << "Cant generate key image for tx: "
                         << cryptonote::get_transaction_hash(tx)
                         << endl;

                    return 1;
                }

                // push to global vector to use it later for checking
                // for our spend_transactions
                key_images.push_back(key_image);

                cout << ", key_image: " << key_image;

                // if so, then add the xmr amount to the money_transfered
                money_received += tx.vout[i].amount;
                cout << ", mine key: " << cryptonote::print_money(tx.vout[i].amount) << endl;
            }
            else
            {
                cout << ", not mine key " << endl;
            }
        }

        cout << "\nTotal xmr received: " << cryptonote::print_money(money_received) << endl;

        //
        // check inputs for spend xmr
        //

        // get the total number of inputs in a transaction.
        // some of these inputs might be our be our spendings
        size_t input_no = tx.vin.size();

        // sum amount of xmr sent by us
        // in the given transaction
        uint64_t money_spend {0};

        cout << endl;

        for (size_t i = 0; i < input_no; ++i)
        {

            // get tx input key
            const cryptonote::txin_to_key& tx_in_to_key
                    = boost::get<cryptonote::txin_to_key>(tx.vin[i]);

            // check if the public key image of this input
            // matches any of your key images that were
            // generated for every output that we recieived
            std::vector<crypto::key_image>::iterator it;
            it = find(key_images.begin(), key_images.end(), tx_in_to_key.k_image);

            cout << ""
                 << "Input no: " << i << ", " << tx_in_to_key.k_image;

            if (it != key_images.end())
            {
                // if so, then add the xmr amount to the money_spend
                money_spend += tx_in_to_key.amount;
                cout << ", mine key image: "
                     << cryptonote::print_money(tx_in_to_key.amount)
                     << endl;
            }
            else
            {
                cout << ", not mine key image " << endl;
            }
        }

        cout << "\nTotal xmr spend: " << cryptonote::print_money(money_spend) << endl;


        //
        // Print summary for the current tx
        //

        cout << "\nSummary for tx: " << cryptonote::get_transaction_hash(tx) << endl;

        if (money_received > money_spend)
        {

            uint64_t xmr_diff = money_received - money_spend;

            cout << " - xmr resieved: " << cryptonote::print_money(xmr_diff) << endl;
            total_xmr_balance += xmr_diff;
        }
        else
        {
            uint64_t xmr_diff = money_spend - money_received;
            uint64_t tx_fee = cryptonote::get_tx_fee(tx);

            cout << "- xmr spent: " << cryptonote::print_money(xmr_diff)
                 << " (includes tx fee: " << cryptonote::print_money(tx_fee) << ")"
                 << endl;

            total_xmr_balance -= xmr_diff;
        }
    }


    // print total xmr balance of after all processing all xmr recieved and xmr spend.
    cout << "\nTotal balance: " << cryptonote::print_money(total_xmr_balance) << endl;

    cout << "\nEnd of program." << endl;

    return 0;
}