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
    auto tx_hash_opt  = opts.get_option<string>("txhash");
    auto viewkey_opt  = opts.get_option<string>("viewkey");
    auto spendkey_opt = opts.get_option<string>("spendkey");
    auto bc_path_opt  = opts.get_option<string>("bc-path");


    // default path to monero folder
    // on linux this is /home/<username>/.bitmonero
    string default_monero_dir = tools::get_default_data_dir();

    // the default folder of the lmdb blockchain database
    // is therefore as follows
    string default_lmdb_dir   = default_monero_dir + "/lmdb";

    // get the program command line options, or
    // some default values for quick check
    //string tx_hash_str  = tx_hash_opt  ? *tx_hash_opt  : "41a21e8d242850bb29a66a07c7a243db2062a61e0f4896d187e386ee0b10f66b";
    string tx_hash_str  = tx_hash_opt  ? *tx_hash_opt  : "ead7b392f57311fbac14477c4a50bee935f1dbc06bf166d219f4c011ae1dc398";
    string viewkey_str  = viewkey_opt  ? *viewkey_opt  : "9c2edec7636da3fbb343931d6c3d6e11bcd8042ff7e11de98a8d364f31976c04";
    string spendkey_str = spendkey_opt ? *spendkey_opt : "950b90079b0f530c11801ef29e99618d3768d79d3d24972ff4b6fd9687b7b20c";
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


    // all transactions hashes related to the new wallet
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
            "97ffac124e8215986cfa9f46fb4824d5f366e48cb5e30495a3debb62bac01c06"
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

    uint64_t total_xmr_balance {0};


    // for each transaction
    for (const cryptonote::transaction& tx: txs)
    {
        // get its public key from extras field
        crypto::public_key pub_tx_key = cryptonote::get_tx_pub_key_from_extra(tx);

        if (pub_tx_key == cryptonote::null_pkey)
        {
            cerr << "Cant get public key of tx with hash: "
                 << cryptonote::get_transaction_hash(tx)
                 << endl;

            return 1;
        }


        // public transaction key is combined with our viewkey
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
             << "tx hash          : <" << tx_hash_str << ">\n"
             << "public tx key    : "  << pub_tx_key << "\n"
             << "dervied key      : "  << derivation << "\n" << endl;


        // check each output to see which belong to us
        // and generate key_images for each

        // get the total number of outputs in a transaction.
        size_t output_no = tx.vout.size();

        // sum amount of xmr sent to us
        // in the given transaction
        uint64_t money_received {0};



        // loop through outputs in the given tx
        // to check which outputs our ours. we compare outputs'
        // public keys with the public key that would had been
        // generated for us if we had gotten the outputs.
        // not sure this is the case though, but that's my understanding.
        for (size_t i = 0; i < output_no; ++i)
        {
            // get the tx output public key
            // that normally would be generated for us,
            // if someone had sent us some xmr.
            crypto::public_key pubkey;

            crypto::derive_public_key(derivation,
                                      i,
                                      public_spend_key, // address.m_spend_public_key
                                      pubkey);

            // get tx output public key
            const cryptonote::txout_to_key tx_out_to_key
                    = boost::get<cryptonote::txout_to_key>(tx.vout[i].target);


            cout << "Output no: " << i << ", " << tx_out_to_key.key;

            // check if the output's public key is ours
            if (tx_out_to_key.key == pubkey)
            {

                //
                // generate key_image of this output
                //
                cryptonote::keypair in_ephemeral;

                crypto::derive_public_key(derivation,
                                          i,
                                          public_spend_key,
                                          in_ephemeral.pub);

                crypto::derive_secret_key(derivation,
                                          i,
                                          private_spend_key,
                                          in_ephemeral.sec);

                crypto::key_image key_image;

                crypto::generate_key_image(in_ephemeral.pub,
                                           in_ephemeral.sec,
                                           key_image);

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
        // check for spendings
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

            const cryptonote::txin_v& in = tx.vin[i];

            // get tx input key
            const cryptonote::txin_to_key& tx_in_to_key
                    = boost::get<cryptonote::txin_to_key>(in);

            // check if the public key image of this input
            // matches any of your key images that were
            // genrated for every output that we recieved
            std::vector<crypto::key_image>::iterator it;
            it = find(key_images.begin(), key_images.end(), tx_in_to_key.k_image);

            cout << ""
                 << "Input no: " << i << ", " << tx_in_to_key.k_image;

            if (it != key_images.end())
            {
                // if so, then add the xmr amount to the money_spend
                money_spend += tx_in_to_key.amount;
                cout << ", mine key image: " << cryptonote::print_money(tx_in_to_key.amount) << endl;
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
            cout << " - xmr resieved: " << cryptonote::print_money(money_received - money_spend) << endl;
            total_xmr_balance += money_received - money_spend;
        }
        else
        {
            cout << "- xmr spent: " << cryptonote::print_money(money_spend - money_received);
            cout << "(includes tx fee: " << cryptonote::print_money(cryptonote::get_tx_fee(tx))
                 << ")" << endl;

            total_xmr_balance -= money_spend - money_received;
        }
    }

    cout << "\nTotal balance: " << cryptonote::print_money(total_xmr_balance) << endl;



    cout << "\nEnd of program." << endl;

    return 0;
}