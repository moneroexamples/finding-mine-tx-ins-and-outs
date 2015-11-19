#include <iostream>
#include <string>

#include "src/MicroCore.h"
#include "src/CmdLineOptions.h"
#include "src/tools.h"


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
    string tx_hash_str  = tx_hash_opt  ? *tx_hash_opt  : "65b60b4182f775b1dfc1e2d73351bf63c61ee332f4eefbf385f55ee059863e98";
    string viewkey_str  = viewkey_opt  ? *viewkey_opt  : "fed77158ec692fe9eb951f6aeb22c3bda16fe8926c1aac13a5651a9c27f34309";
    string spendkey_str = spendkey_opt ? *spendkey_opt : "1eaa41781d5f880dc69c9379e281225c781a6db8dc544a26008e7a07890afa03";
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



    cout << "\n"
         << "Private spend key: " << private_spend_key << "\n"
         << "Public spend key : " << public_spend_key  << endl;

    cout << "\n"
        << "Private view key : "  << private_view_key << "\n"
        << "Public view key  : "  << public_view_key  << endl;


    cout << "\n"
         << "Monero address   : "  << address << endl;




    // get the highlevel cryptonote::Blockchain object to interact
    // with the blockchain lmdb database
    cryptonote::Blockchain& core_storage = mcore.get_core();


    // we also need tx public key, but we have tx hash only.
    // to get the key, first, we obtained transaction object tx
    // and then we get its public key from tx's extras.
    // this is done using get_tx_pub_key_from_str_hash function
    cryptonote::transaction tx;

    if (!xmreg::get_tx_pub_key_from_str_hash(core_storage, tx_hash_str, tx))
    {
        cerr << "Cant find transaction with hash: " << tx_hash_str << endl;
        return 1;
    }


    crypto::public_key pub_tx_key = cryptonote::get_tx_pub_key_from_extra(tx);

    if (pub_tx_key == cryptonote::null_pkey)
    {
        cerr << "Cant get public key of tx with hash: " << tx_hash_str << endl;
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


    // each tx that we (or the address we are checking) received
    // contains a number of outputs.
    // some of them are ours, some not. so we need to go through
    // all of them in a given tx block, to check which outputs are ours.

    // get the total number of outputs in a transaction.
    size_t output_no = tx.vout.size();

    // sum amount of xmr sent to us
    // in the given transaction
    uint64_t money_transfered {0};

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
            // if so, then add the xmr amount to the money_transfered
            money_transfered += tx.vout[i].amount;
            cout << ", mine key: " << cryptonote::print_money(tx.vout[i].amount) << endl;
        }
        else
        {
            cout << ", not mine key " << endl;
        }
    }

    cout << "\nTotal xmr received: " << cryptonote::print_money(money_transfered) << endl;




    cryptonote::account_keys acc_keys {address, private_spend_key, private_view_key};

    std::unordered_map<crypto::key_image, size_t> key_images;


    // public transaction key is combined with our spendkey
    // to create, so called, derived key.
    crypto::key_derivation recv_derivation;

    if (!generate_key_derivation(pub_tx_key, private_view_key, recv_derivation))
    {
        cerr << "Cant get dervied key for: " << "\n"
             << "pub_tx_key: " << private_view_key << " and "
             << "private_key" << private_spend_key << endl;
        return 1;
    }



    // generate key images to be used for checking inputs for spending
    for (size_t i = 0; i < output_no; ++i)
    {
        cryptonote::keypair in_ephemeral;


        crypto::derive_public_key(recv_derivation,
                                  i,
                                  public_spend_key,
                                  in_ephemeral.pub);


        crypto::derive_secret_key(recv_derivation,
                                  i,
                                  private_spend_key,
                                  in_ephemeral.sec);
//

        crypto::key_image key_image2;

        crypto::generate_key_image(in_ephemeral.pub,
                                   in_ephemeral.sec,
                                   key_image2);

        crypto::key_image key_image;
        cryptonote::keypair in_ephemeral2;
        cryptonote::generate_key_image_helper(acc_keys,
                                              pub_tx_key,
                                              i,
                                              in_ephemeral2,
                                              key_image);



        cout << "Key image for output " << i << ": " << key_image << endl;
        cout << "Key image for output " << i << ": " << key_image2 << endl;

        key_images[key_image2] = i;

    }



    // get the total number of outputs in a transaction.
    size_t input_no = tx.vin.size();

    // sum amount of xmr sent by us
    // in the given transaction
    uint64_t money_received {0};

    // loop through inputs in the given tx
    // to check which inputs our ours.
    for (size_t i = 0; i < input_no; ++i)
    {



        cryptonote::txin_v& in = tx.vin[i];

       // if(in.type() != typeid(cryptonote::txin_to_key))
         //   continue;


        // get tx input public key
        const cryptonote::txin_to_key tx_in_to_key
                = boost::get<cryptonote::txin_to_key>(in);





        auto it = key_images.find(tx_in_to_key.k_image);


        for (auto& kimg: key_images)
        {
            cout <<  kimg.second << kimg.first << endl;
        }


        cout << "\n"
             << "Input no: " << i << ", " << tx_in_to_key.k_image;

        if (it != key_images.end())
        {
            // if so, then add the xmr amount to the money_spend
            cout << ", mine key: " << cryptonote::print_money(tx_in_to_key.amount) << endl;
        }
        else
        {
            cout << ", not mine key " << endl;
        }


    }




    cout << "\nEnd of program." << endl;

    return 0;
}