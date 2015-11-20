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



       // cout << "Key image for output " << i << ": " << key_image << endl;
        //cout << "Key image for output " << i << ": " << key_image2 << endl;

        key_images[key_image2] = i;

    }





    // hardcode output key images from previous transactions
    // easier to test, than to search the blockchain each time


    vector<string> past_key_images_str {
            "b45393312d5f9c28d043aa4ad9ba76a16db70a234b2bce95b9ff768a63c43dde",
            "18f6f75174d1c79dec18bbbfb86004d3000750871040af35e69ea8bd536af2fe",
            "38619bfb8c3c57beed613c2c46d3bb315987bb1f1b32a31c3d30d4283180b0f0",

            "2bb5817d8785e712e99969d36bfd9a956b2fa5b95442ae95fdd6cebf7d1b78a4",
            "9f99cef8b4df531b1724767f088c57e808ebd43dfaa18d926d50cc1dcbb16b8c",
            "eb0961e82846358dc2a648d4871dfd8ed0aaf6a123c321a63fc9e1d3312efb8d",

            "549782c659cd85dd2642d14936e2557cc17b4fecd8b2b766e774fa2404c30e49",
            "359e6f9ad4fd529c6f3ba44af7b6420fd0f002aac8d3a8828edab1bb2fbb11d1",

            "66ddbf99b6e30f0bf4aa284600ede73019ee0866d7ecf2f2a11d39fd7511ff43",
            "be1683c4b11cb66787c27cd31ce86b57a029ee9201bc9da5fbff32f1c767565a",
            "18079f7b8ba89ae6725c791f4edddd9fd2f6c46f36bed9b21b512a846e05e38b",

            "8618f33ebcb9b13e93eda2d7249d32b7e522b3b8ea706dddffd7acf38d38248f",
            "70a837de5fefc5284984effe97cbe0159ba90e3464dec48803ffd952eafc2cb9",

            "b75fb25b08a55572284e4312c658b772617dec74fc603d764d47627fa1cef6f5",
            "ad3b9a85e6ed25bce75af4dca670ee564a4ba74b58c8289ad10c286df2241a99",

            "ae0b3e22bf5dca03da1b82848f120049b51f1b0f3dbe40e76ca5d5ba796ac1d5",
            "c46c77bfecee9f2ecdda53f3c1635c8a901c561dee048180d18455b97374a1a1",

            "682226e7bb46aa71f64c5de90b34d0bf93caa947831e3f64a66c134e0b3b1fe4",
            "84bd11a888a0666e62f989ad1282c7f1ec107a744d09f0f55f71394e85da400a"
    };


    std::unordered_map<crypto::key_image, size_t> key_images2;


    for (string& key_image_str: past_key_images_str)
    {
        crypto::key_image ki;
        xmreg::parse_str_secret_key(key_image_str, ki);
        key_images2[ki] = 0; // <- some temp value for now
    }




    // get the total number of outputs in a transaction.
    size_t input_no = tx.vin.size();

    // sum amount of xmr sent by us
    // in the given transaction
    uint64_t money_spend {0};

    cout << endl;

    // loop through inputs in the given tx
    // to check which inputs our ours.
    for (size_t i = 0; i < input_no; ++i)
    {



        cryptonote::txin_v& in = tx.vin[i];

       // if(in.type() != typeid(cryptonote::txin_to_key))
         //   continue;


        // get tx input public key
        const cryptonote::txin_to_key& tx_in_to_key
                = boost::get<cryptonote::txin_to_key>(in);


        auto it = key_images2.find(tx_in_to_key.k_image);

        cout << ""
             << "Input no: " << i << ", " << tx_in_to_key.k_image;

        if (it != key_images.end())
        {
            // if so, then add the xmr amount to the money_spend
            money_spend += tx_in_to_key.amount;
            cout << ", mine key: " << cryptonote::print_money(tx_in_to_key.amount) << endl;
        }
        else
        {
            cout << ", not mine key " << endl;
        }
    }

    cout << "\nTotal xmr spend: " << cryptonote::print_money(money_spend) << endl;



    uint64_t received = (money_spend < money_transfered)
                        ? money_transfered - money_spend
                        : 0;

    if (money_spend < money_transfered)
    {
        cout << "\n Xmr resieved: " << cryptonote::print_money(money_transfered - money_spend) << endl;
    }
    else
    {
        cout << "\n Xmr spent: " << cryptonote::print_money(money_spend - money_transfered) << endl;
    }









    cout << "\nEnd of program." << endl;

    return 0;
}