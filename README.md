# Find which tx inputs and ouputs are ours

Due to how [Monero](https://getmonero.org/) monero works, it is rather
impossible to know which input and outputs in a given transaction belong
to a specific user based on their record in the blockchain. The reason is, that knowing
someone's xmr address
does not enable us to get this information from the blockchain. In order
to obtain this information, private view and spend
keys are required. But how do you use them to get this information?

One way is to restore your wallet using your mnemonic seed. During
the restoration process, a log file will be generated containing
found incoming and outcoming transactions. Another way is to write your own
program for this.

How this can be done, it is demonstrated in this
example. Specifically, the example shows how check which inputs and outputs
in transactions belong to a given users (knowing private spend and view keys) in C++.

To make this example execute fast and simple, I created a test wallet and made a number of
incoming and outcoming transactions to and from the wallet. Each
transaction was recorded manually for the verification of the results.

In order to avoid scanning the blockchain for transactions, I hard coded
transaction hashes in the code. Another example will show how to do it in
a more general way.

The full record of the transactions is here:

 - [manual record](https://github.com/moneroexamples/finding-mine-tx-ins-and-outs/blob/master/tx_manual_record.txt)
 - [simplewallet restoration log](https://github.com/moneroexamples/finding-mine-tx-ins-and-outs/blob/master/tx_restore_log.txt)


## Pre-requisites

Everything here was done and tested on
Ubuntu 14.04 x86_64 and Ubuntu 15.10 x86_64.

Instruction for Monero compilation:
 - [Ubuntu 14.04 x86_64](http://moneroexamples.github.io/compile-monero-ubuntu/)
 - [Ubuntu 15.10 x86_64](http://moneroexamples.github.io/compile-monero-ubuntu-1510/)


Monero source code compilation and setup are same as [here](http://moneroexamples.github.io/access-blockchain-in-cpp/).



# C++ code
The main part of the example is main.cpp.

## main.cpp


```c++
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


    // the default folder of the lmdb blockchain database
    string default_lmdb_dir   = xmreg::get_default_lmdb_folder();


    // language for generation of mnemonic seed
    string language {"English"};

    // for this example, I made new wallet. These are the transaction hashes
    // that were send to the wallet, or send from the wallet. I hardcoded them here,
    // because its was much easier to work on the example. For more general
    // use, one would have to scan the blockchain to determine which
    // transactions our ours. This will be probably another example.
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

    // hardcoded private view and spend keys for the new wallet created.
    // this example will work only with these keys and the corresponding
    // tx hashes. To have general program which will work with any private keys,
    // scanning the blockchain is required, because without this, it is not
    // possible to know which transaction outputs and inputs are associated
    // with the keys. This probably will be another example.
    string viewkey_str  = "9c2edec7636da3fbb343931d6c3d6e11bcd8042ff7e11de98a8d364f31976c04";
    string spendkey_str = "950b90079b0f530c11801ef29e99618d3768d79d3d24972ff4b6fd9687b7b20c";


    // get the program command line options, or default values
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


    // we have private_view_key, so now
    // we need to get the corresponding
    crypto::public_key public_view_key;


    // generate public key based on the private key
    crypto::secret_key_to_public_key(private_view_key, public_view_key);


    // parse string representing given monero address
    cryptonote::account_public_address address {public_spend_key, public_view_key};


    // 25 word mnemonic that is provided by the simplewallet
    string mnemonic_str;

    // derive the mnemonic version of the spend key.
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



    // create instance of our MicroCore
    xmreg::MicroCore mcore;

    // initialize the core using the blockchain path
    if (!mcore.init(blockchain_path.string()))
    {
        cerr << "Error accessing blockchain." << endl;
        return 1;
    }

    // get the high level cryptonote::Blockchain object to interact
    // with the blockchain lmdb database
    cryptonote::Blockchain& core_storage = mcore.get_core();

    vector<cryptonote::transaction> txs;

    // populate transactions vector using
    // tx hashes provided above
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
    // and our private spend key.
    // this is the most tricky part of the example.
    // the reason is that by simply looking at individual transactions
    // it is not possible to know which inputs are ours even if
    // we have private view and spend keys. We can do this with outputs
    // but inputs. So how do you know which outputs in a given transactions
    // are ours? The answer is that we need to keep track of all our
    // previous outputs. In other words, the key_image listed in inputs
    // of a given transaction will correspond (if they belongs to us)
    // to some key images derived from our past outputs
    vector<crypto::key_image> key_images;

    // total xmr balance
    uint64_t total_xmr_balance {0};

    // transaction index
    size_t tx_index {0};


    // for each transaction, go through its outputs and inputs.
    // for each output, check if it belongs to us, based
    // on our private view key. If so, then get the xmr amount
    // sent to us, and also generate key image for this output.
    // key images are generated using our public spend key. each generated
    // key image, is stored in a key_images vector.
    //
    // after we are done with outputs, we go to check inputs. inputs
    // are our spendings, but which input is ours? for this, we need
    // to check if input's key_image, matches any of ours key_images.
    // if there is a match, it means that this input is ours, i.e.,
    // we sent xmr somewhere.
    //
    // When we spend xmr, inputs used will add up to no less than
    // what we spend. Thus, if they they are more than what we spend
    // we will get back a change in the outputs of the current transaction.
    for (const cryptonote::transaction& tx: txs)
    {
        cout << "\n\n"
             << "********************************************************************\n"
             << "Transaction: " << ++tx_index << "\n"
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


        // public transaction key is combined with our private view key
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
             << "derived key      : "  << derivation << "\n"
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
        // to check which outputs our ours. for each our output,
        // we get xmr amount sent to us, and we generate its key image.
        // the key images generated stored in
        // the global key_images vector.
        for (size_t i = 0; i < output_no; ++i)
        {
            // get the tx output public key
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
                // for our inputs (i.e. spend xmr)
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
            // generated for every output that we received
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

            total_xmr_balance += xmr_diff;

            cout << " - xmr received: " << cryptonote::print_money(xmr_diff) << endl;
        }
        else
        {
            uint64_t xmr_diff = money_spend - money_received;

            // get tx fee
            uint64_t tx_fee = cryptonote::get_tx_fee(tx);

            total_xmr_balance -= xmr_diff;

            cout << "- xmr spent: " << cryptonote::print_money(xmr_diff)
                 << " (includes tx fee: " << cryptonote::print_money(tx_fee) << ")"
                 << endl;
        }

        cout << "\nAfter this tx, total balance is: "
             << cryptonote::print_money(total_xmr_balance)
             << endl;
    }


    // print total xmr balance of after all processing all xmr received and xmr spend.
    cout << "\nFinal total balance: " << cryptonote::print_money(total_xmr_balance) << endl;

    cout << "\nEnd of program." << endl;

    return 0;
}
```

# Output of the example

```bash
Private spend key: <950b90079b0f530c11801ef29e99618d3768d79d3d24972ff4b6fd9687b7b20c>
Public spend key : <287e4f75723d62f0fab06a58893535adc8082f7834c462f30158cecc3e8af327>

Private view key : <9c2edec7636da3fbb343931d6c3d6e11bcd8042ff7e11de98a8d364f31976c04>
Public view key  : <6cc88f08944d5f3b4f811ae011436fbcadc668b566883ce34d06395f450288e4>

Monero address   : <43A7NUmo5HbhJoSKbw9bRWW4u2b8dNfhKheTR5zxoRwQ7bULK5TgUQeAvPS5EVNLAJYZRQYqXCmhdf26zG2Has35SpiF1FP>

Mnemonic seed    : hookup hijack imagine touchy audio bowling gnaw scenic rapid oncoming shrugged gang fazed unhappy lumber amply altitude duties ozone silk hashing feel tolerant uptight tolerant
[1;32m2015-Nov-22 08:59:57.010836 Blockchain initialized. last block: 836295, d0.h0.m2.s4 time ago, current difficulty: 813369436
[0m

********************************************************************
Transaction: 1
********************************************************************

tx hash          : <ead7b392f57311fbac14477c4a50bee935f1dbc06bf166d219f4c011ae1dc398>
public tx key    : <8957a240ce4802637d38d8e12d31a3f8d27dc9f5c2713395419b95e422cfe088>
derived key      : <928d282ead4b330ff4aaaa7b505e2cbbb2be219bc9239a7d5ad6a618d83d2841>

Output no: 0, <ae8c1e5c9aff17e29ed6f25fc69120f5356c065140da52c17af97ef6595ba2db>, key_image: <d89cc46e14ac6251b811964735b8fff82dc6b5534339d137d65144e8c78e798a>, mine key: 0.040000000000
Output no: 1, <7b96c0bb8c177b485f99b84d2d446d480e6584de6d3a38c69dd32e0d480e673a>, key_image: <dafab55eead3b665f8db0b86e5c3d87320bbf3e05624539224f0316d76d377b0>, mine key: 0.200000000000
Output no: 2, <b9612c9ce5924816b12ca59a6225d4fc86aaec26041a56b1fb82c1d6897beeb3>, key_image: <1b5baf945a81ac4c48add88af115ca9481ee0647f79d813ae63f8ed2cec54afc>, mine key: 2.000000000000

Total xmr received: 2.240000000000

Input no: 0, <84bd11a888a0666e62f989ad1282c7f1ec107a744d09f0f55f71394e85da400a>, not mine key image
Input no: 1, <682226e7bb46aa71f64c5de90b34d0bf93caa947831e3f64a66c134e0b3b1fe4>, not mine key image
Input no: 2, <b75fb25b08a55572284e4312c658b772617dec74fc603d764d47627fa1cef6f5>, not mine key image
Input no: 3, <ae0b3e22bf5dca03da1b82848f120049b51f1b0f3dbe40e76ca5d5ba796ac1d5>, not mine key image
Input no: 4, <ad3b9a85e6ed25bce75af4dca670ee564a4ba74b58c8289ad10c286df2241a99>, not mine key image

Total xmr spend: 0.000000000000

Summary for tx: <ead7b392f57311fbac14477c4a50bee935f1dbc06bf166d219f4c011ae1dc398>
 - xmr received: 2.240000000000

After this tx, total balance is: 2.240000000000


********************************************************************
Transaction: 2
********************************************************************

tx hash          : <50a3ded2df473a7e8a7fde58c8a865d1ae246ce8ceddb5f474164888fe2ad822>
public tx key    : <a22930bfbdbd51f6d40932789f4914297ed12da5d73e9bc621f56bba68bae49e>
derived key      : <c59ab95a0b2cceb80bf956168816fdc2551814f49a558e53b01aa7dd9e4ae947>

Output no: 0, <cd7064fc8fb5bd308eb205b8600db0ed34302731e7a02f5097bd61878b0df07a>, key_image: <288844e10a669b902e201f610d5c8283b2ae1e7fb1bc9562f879c0a93d5161d2>, mine key: 0.001000000000
Output no: 1, <bf1e2acf806b89778fc545d610f779903b2b0630dc0fc35dd900a8b6098839cc>, key_image: <adc186efd48246730a688fba5d8aacb93c94650e38b8af7a84583d621077b3b7>, mine key: 0.020000000000
Output no: 2, <79a85dc242d70814d0659f1b738d66c515a2d0f9e42d6119b985585cf851ee3e>, not mine key
Output no: 3, <3e77a6dd2b22ff31b5ed9ef472d2bdec9e69f3650c62f6487dffa3b099533f51>, key_image: <8fbd85d15d4517a222c2c2ec04ca3325d12fc7e8a9437cb8929e2acf533c135d>, mine key: 0.300000000000
Output no: 4, <9a1e997390e116913842363bda1f174b65e87636520b613b62650c831842c263>, not mine key
Output no: 5, <b4b2a14faf34dc3ecf6b30784aab67af8e8cb62bdae099c5654a29903413776b>, key_image: <920b03fc4888db8dc4b9893d1e4dc1afdceedefadfb4a25fd09b5c2bfc86c4c8>, mine key: 1.000000000000

Total xmr received: 1.321000000000

Input no: 0, <191f5fd4e003ab60b28babe46f7ae96a858aed3ed2077324a5edda8308698915>, not mine key image
Input no: 1, <d2624bdf67cb9336df445b6b98485175ebb0e836265b7afb942f4ce19b41c530>, not mine key image

Total xmr spend: 0.000000000000

Summary for tx: <50a3ded2df473a7e8a7fde58c8a865d1ae246ce8ceddb5f474164888fe2ad822>
 - xmr received: 1.321000000000

After this tx, total balance is: 3.561000000000


********************************************************************
Transaction: 3
********************************************************************

tx hash          : <fed715d3361f3c66437e1e19f193c93abb86c74b5d77ea464e95f27c37097214>
public tx key    : <dec5a7781c2c2feb86fb29dc9b3eb4966f419dffc9326b5ea6db17d8de4293dc>
derived key      : <d4e7615b9a2382c323b42bdc569826f15276e3a7c715f0ebd8b719d2706d77ec>

Output no: 0, <b92d301691da0b0e35a0aa5321171b1c003ec3b02faaf0e639c0867559d0641d>, not mine key
Output no: 1, <c15dbf1fbfb7fd59fea0fefc9cc0a19e7826232f7b181cbc64baef498ef4e30f>, not mine key
Output no: 2, <614b2a2e834a432582d7ca06fd35f8d2f7b8121961e8c5a5071064ae959d2831>, key_image: <74569718678d07f962111300bde7facb40d5633852a1ac8db07983a4b537528b>, mine key: 0.600000000000
Output no: 3, <38e48d0a974d5edeca5a9ab10ccc232b7097593be89eeaef80c29099ecdd608e>, not mine key

Total xmr received: 0.600000000000

Input no: 0, <3791fe892209d98b33484fae44f6fa7999aa3e7cba9bc475bc1f206037a5f455>, not mine key image

Total xmr spend: 0.000000000000

Summary for tx: <fed715d3361f3c66437e1e19f193c93abb86c74b5d77ea464e95f27c37097214>
 - xmr received: 0.600000000000

After this tx, total balance is: 4.161000000000


********************************************************************
Transaction: 4
********************************************************************

tx hash          : <948a7ce9971d05e99a43f35e11e4c6a346e7d2b71758bed5cb4f9fc175f7bb5f>
public tx key    : <b440bd6091b051c2c50873abf92a43fd5d59906c7d2f557dc4472f555da50dce>
derived key      : <357c721a0546d9c4519631be4ee37172c6355943a91422fa9fe0b4b6411c8276>

Output no: 0, <8732f488c43c6fbc0faca3543b8b01c861cc6faf61c374234a71641e80358877>, not mine key
Output no: 1, <267ab2efd5a8c4c1c0995dd8c9aff02ee9d67f428ed70e86301380f1756afee9>, key_image: <fb3977056cf13a58d92e688cf3541c4aa9bf79c7f9e3c03323e3b2c94358bde0>, mine key: 0.200000000000
Output no: 2, <f9e50fee7ebb6609fc241729e0f241d8c811e9f9f21790ea21dd2cd2e28dd50e>, not mine key
Output no: 3, <642593e8a96ac61e6972da338cb29cdf54a059478fcf7cb492bbf15ba8229e2d>, key_image: <f2f2c2a10bcd24630b47623fbcb06c674e8b1fce41e19e87a6c45cd3412fbf83>, mine key: 1.000000000000

Total xmr received: 1.200000000000

Input no: 0, <1b5baf945a81ac4c48add88af115ca9481ee0647f79d813ae63f8ed2cec54afc>, mine key image: 2.000000000000

Total xmr spend: 2.000000000000

Summary for tx: <948a7ce9971d05e99a43f35e11e4c6a346e7d2b71758bed5cb4f9fc175f7bb5f>
- xmr spent: 0.800000000000 (includes tx fee: 0.020000000000)

After this tx, total balance is: 3.361000000000


********************************************************************
Transaction: 5
********************************************************************

tx hash          : <edc9671f6f988f7d9a5265c1f7829d5b8ecc2f6ddafa995b23a1dd04f7834713>
public tx key    : <5eeb03e1d30bfdd669f9d2bec8268d9ba00e4ccb21ab3a67483d41bfee95177e>
derived key      : <9bb536d1e88cdd1ffe3befadbab99517dd7ebca98860237c6e46af6e836e3310>

Output no: 0, <77fbe73e43a5842b35bc823ba9da685bb0aba962f1bc44ed407a06635e2f257d>, key_image: <e1b2c16fe7bcc0cfc68b37d1569b086f74baa7a47a248620713148f137e077af>, mine key: 0.040000000000
Output no: 1, <67113311c4140a2552fe1766ece7e3e78d0c852443fafd83ce4a342b6b437828>, not mine key
Output no: 2, <eab4307831c467c35fc05f239641906ab0a4214ca94f81d9e21d6da88d4f2854>, not mine key
Output no: 3, <95611d583f8a15bedc54b4e7b396724a554ad8e203472939b2e3a115a6a872a3>, key_image: <e2869f28c913829e6a48b66c2b8a01341f2e6beb4eeec6ace50f03be9567eca6>, mine key: 0.100000000000

Total xmr received: 0.140000000000

Input no: 0, <8fbd85d15d4517a222c2c2ec04ca3325d12fc7e8a9437cb8929e2acf533c135d>, mine key image: 0.300000000000

Total xmr spend: 0.300000000000

Summary for tx: <edc9671f6f988f7d9a5265c1f7829d5b8ecc2f6ddafa995b23a1dd04f7834713>
- xmr spent: 0.160000000000 (includes tx fee: 0.010000000000)

After this tx, total balance is: 3.201000000000


********************************************************************
Transaction: 6
********************************************************************

tx hash          : <7132cd214d9b7b502b2990b61181ba1cde203b9e4c648d60a9772e56c1ff2980>
public tx key    : <155b29addb23dda4ea196f27b54ac334fc9f5b108f5a2e5b65c79106eec4ddf1>
derived key      : <118cfe2ca91e3e8ac8453723c338a8a5f3d9accb3511299f7e1b19e5aa780cd9>

Output no: 0, <3c97b475e528203c4fc322553c04e7602f8de48d2c456fd3cac90e87020a4bce>, not mine key
Output no: 1, <551314d9779c8431c22b1433ce0fb84fe782ccc4edc06c275cb9d62e7acde295>, not mine key
Output no: 2, <0857ea979bbf2f81daf4519d76931ec46806330e8f61b1f03eef0ef3ebed9462>, key_image: <733fa04a2c009f8f0818de09094ecab485a47e275b7e543968b22cff0550b51a>, mine key: 0.600000000000
Output no: 3, <4273441c40167cfe947fc375e17973f4b36f050896594ce104f4bae9ad482bf8>, key_image: <24e151d0c30fd941976b74a832bfdc5e4fc1df50f833e0c0bb41fff1b228ce15>, mine key: 1.000000000000
Output no: 4, <7f9345335ed785df09703c49a310bad97fc489718c5d88f451040eff88975b70>, not mine key

Total xmr received: 1.600000000000

Input no: 0, <47d31161b691222cc54819167be55adbd7d8fc4bd6299c78275b531729fbe6c2>, not mine key image
Input no: 1, <df9e084138d1f0492ba1ad25369617ed1bf8925ba4a64cfb2b519184e105dc46>, not mine key image

Total xmr spend: 0.000000000000

Summary for tx: <7132cd214d9b7b502b2990b61181ba1cde203b9e4c648d60a9772e56c1ff2980>
 - xmr received: 1.600000000000

After this tx, total balance is: 4.801000000000


********************************************************************
Transaction: 7
********************************************************************

tx hash          : <60465baab286ae6656378ab5b036b32c238347e7d83988be6d613c17dc586cc4>
public tx key    : <961422b450774f1818293152f0598f4f83d06888007f098c98bdc7fbbc7c34a7>
derived key      : <aa8aae66ee24facdcf1c5258a59fd943af93830e26e7e1c646ae1f726b088ddb>

Output no: 0, <880a1912b4246936615efc544af896b1f89e0164a2e8b4a31ce1f93455df5194>, key_image: <c2879575f53e900d764e911270be6c2dcd1638b664075606af7f49bad3f7bfe8>, mine key: 0.090000000000
Output no: 1, <68a67913ae18d9724950379dacf14983c10ff6213982f1828bb009007fceb540>, key_image: <94b4dddd1dc71cde7038c8f9495a4f618d9c639f4d8ab2c5b2c4e8b24b38f2c6>, mine key: 0.200000000000
Output no: 2, <25244c0896ca02a569c97d264a54827b84f47abf818c3078be4a972625a6f63c>, not mine key
Output no: 3, <afcbdbb0057be5494ff0066341db62654501b9a490257e7ac262bb21ea50ef7b>, not mine key

Total xmr received: 0.290000000000

Input no: 0, <74569718678d07f962111300bde7facb40d5633852a1ac8db07983a4b537528b>, mine key image: 0.600000000000
Input no: 1, <e1b2c16fe7bcc0cfc68b37d1569b086f74baa7a47a248620713148f137e077af>, mine key image: 0.040000000000
Input no: 2, <dafab55eead3b665f8db0b86e5c3d87320bbf3e05624539224f0316d76d377b0>, mine key image: 0.200000000000
Input no: 3, <e2869f28c913829e6a48b66c2b8a01341f2e6beb4eeec6ace50f03be9567eca6>, mine key image: 0.100000000000
Input no: 4, <f2f2c2a10bcd24630b47623fbcb06c674e8b1fce41e19e87a6c45cd3412fbf83>, mine key image: 1.000000000000

Total xmr spend: 1.940000000000

Summary for tx: <60465baab286ae6656378ab5b036b32c238347e7d83988be6d613c17dc586cc4>
- xmr spent: 1.650000000000 (includes tx fee: 0.050000000000)

After this tx, total balance is: 3.151000000000


********************************************************************
Transaction: 8
********************************************************************

tx hash          : <5dcc5eb9cd89f8d364f7ea4ad789067f8c16425ce94d6a6e38e1c0c3f1fedae6>
public tx key    : <f1358b655058056c648af441ffc778d85841424c67048a7a79cff1eccd2de4e9>
derived key      : <beecc331f3f9e3ce8fa8df7d7776cb4bc7eb6edc0865c519a26a418aef41d85e>

Output no: 0, <7ada581b733fb060dc01baac06ea37e22cc8af53292e06a349b0ea74091e0c24>, not mine key
Output no: 1, <d3da7abd3675a1bf42f38dade1b976b38e593c02600aa2cb60ece6fef209570f>, key_image: <484761e7b204a70f908493761b02b25cb472ab1a3506a56c438b1e6306e675eb>, mine key: 0.100000000000
Output no: 2, <9a8fdb88338dbad73d46142b98dcb7e187ebee4ca9b7acad3507556b6157d5c6>, key_image: <e5f0afc9a60012d8b408b5decede24987fd93d5d10155be840e7b4851d0df549>, mine key: 1.000000000000

Total xmr received: 1.100000000000

Input no: 0, <7fc874157d2bd9a81809a4501009fe15514d39376bd441e236daeef3fcc0a534>, not mine key image
Input no: 1, <ac5b53601d78870e0e0334a0e3e0cc095280135806b0d6111399e3d84d8f4a4b>, not mine key image
Input no: 2, <612041bdc0d97c0c730c82f1e470996fcc53685be63e43987efd0955de01bc1d>, not mine key image

Total xmr spend: 0.000000000000

Summary for tx: <5dcc5eb9cd89f8d364f7ea4ad789067f8c16425ce94d6a6e38e1c0c3f1fedae6>
 - xmr received: 1.100000000000

After this tx, total balance is: 4.251000000000


********************************************************************
Transaction: 9
********************************************************************

tx hash          : <6f6d97eaa2de50d27b60ce8ac40b0b8dd53a56f7d9f17d81a71b29194d53dd58>
public tx key    : <366658f0488f59b0d125b1abdb0f72dd22fbf4e4ebb06bc1dad70bf3d383eb87>
derived key      : <e988869eb26609f0dc89aef61e1dae25e90104f12ae7646342facb1f8067154c>

Output no: 0, <d4d9708def5210785fcf5401d5caa6fcf798705760b9e8cb7b6d24e731afd222>, key_image: <7b82eca9b1921b5d4e868831f87b1864b0b3850d03e79e72d856ad566b22c7d3>, mine key: 0.020000000000
Output no: 1, <c2342d18d1dbf517d52a498c08019c40873a46c159eeddc01b7ee1fad65a0bad>, not mine key
Output no: 2, <a6fd15e65012fe0cacb8aae39b2396657d2c5747bdb10a25aeffa92998ae8a7c>, not mine key

Total xmr received: 0.020000000000

Input no: 0, <adc186efd48246730a688fba5d8aacb93c94650e38b8af7a84583d621077b3b7>, mine key image: 0.020000000000
Input no: 1, <fb3977056cf13a58d92e688cf3541c4aa9bf79c7f9e3c03323e3b2c94358bde0>, mine key image: 0.200000000000
Input no: 2, <920b03fc4888db8dc4b9893d1e4dc1afdceedefadfb4a25fd09b5c2bfc86c4c8>, mine key image: 1.000000000000
Input no: 3, <d89cc46e14ac6251b811964735b8fff82dc6b5534339d137d65144e8c78e798a>, mine key image: 0.040000000000

Total xmr spend: 1.260000000000

Summary for tx: <6f6d97eaa2de50d27b60ce8ac40b0b8dd53a56f7d9f17d81a71b29194d53dd58>
- xmr spent: 1.240000000000 (includes tx fee: 0.040000000000)

After this tx, total balance is: 3.011000000000


********************************************************************
Transaction: 10
********************************************************************

tx hash          : <97ffac124e8215986cfa9f46fb4824d5f366e48cb5e30495a3debb62bac01c06>
public tx key    : <04fe0d0c16b681e2b4b99dfa49ef1e2e01e571b5693a66e41f865f59650b4565>
derived key      : <15426273bb93b32e16e2659a986fa560cbc28fc1aa538bf823d7ae2b6b4f5aa7>

Output no: 0, <afa136048deb45567183e08f3e40e710d4ce6bb1b817108240bd634a6cb34993>, not mine key
Output no: 1, <788c61f43986f4290d7fe334586df70bf349c261f56f2b6bbbad82f723d11947>, not mine key
Output no: 2, <0bb9a039f24c3adade36d6a391470e6925a41eca003ca5310855d9abbe110245>, not mine key
Output no: 3, <d12fa7c79532dec4c0ed850d8c200ffdd9f8671aeb4b8413569dc512345372b6>, not mine key

Total xmr received: 0.000000000000

Input no: 0, <e5f0afc9a60012d8b408b5decede24987fd93d5d10155be840e7b4851d0df549>, mine key image: 1.000000000000
Input no: 1, <c2879575f53e900d764e911270be6c2dcd1638b664075606af7f49bad3f7bfe8>, mine key image: 0.090000000000
Input no: 2, <94b4dddd1dc71cde7038c8f9495a4f618d9c639f4d8ab2c5b2c4e8b24b38f2c6>, mine key image: 0.200000000000
Input no: 3, <7b82eca9b1921b5d4e868831f87b1864b0b3850d03e79e72d856ad566b22c7d3>, mine key image: 0.020000000000
Input no: 4, <733fa04a2c009f8f0818de09094ecab485a47e275b7e543968b22cff0550b51a>, mine key image: 0.600000000000
Input no: 5, <484761e7b204a70f908493761b02b25cb472ab1a3506a56c438b1e6306e675eb>, mine key image: 0.100000000000
Input no: 6, <24e151d0c30fd941976b74a832bfdc5e4fc1df50f833e0c0bb41fff1b228ce15>, mine key image: 1.000000000000
Input no: 7, <288844e10a669b902e201f610d5c8283b2ae1e7fb1bc9562f879c0a93d5161d2>, mine key image: 0.001000000000

Total xmr spend: 3.011000000000

Summary for tx: <97ffac124e8215986cfa9f46fb4824d5f366e48cb5e30495a3debb62bac01c06>
- xmr spent: 3.011000000000 (includes tx fee: 0.030000000000)

After this tx, total balance is: 0.000000000000


********************************************************************
Transaction: 11
********************************************************************

tx hash          : <6b524290bd6a955e72ad47d88736a871f7c2661f225e1127f4310c00e585f974>
public tx key    : <9125488004230e48cc6cf29c4ee326c64bde8b74f54c84ec6b9aaf6eb4e17fea>
derived key      : <0b029a8dffd54f9e629be79fb0e9d0a25d14a91b262f281c466bf04a3b091717>

Output no: 0, <dab149062e5ef92488a2f7acd4adb9c1b2d163764668a6a0fa150361fea061bf>, key_image: <e4235e76bb0c45cb29dd633616f489193457262da9dcc23859a036a1378f9505>, mine key: 0.080000000000
Output no: 1, <ea818050d8a974009e02afeb9c9ccb0947a8b2ce0cce68bc8bc90377b66e14de>, key_image: <6c3eff3d76c78088fd2e80be9da13a7cbfe20e6f3482847f621e8194ff361e53>, mine key: 0.400000000000

Total xmr received: 0.480000000000

Input no: 0, <ddd4a31cd132362e228f6ce7fb0fca64088b6011f45c4436836e55c9521d22c5>, not mine key image
Input no: 1, <ee30f5e671097b2f94a8776f82e8ade5ca43468cc1570933d8a2782999ad7436>, not mine key image

Total xmr spend: 0.000000000000

Summary for tx: <6b524290bd6a955e72ad47d88736a871f7c2661f225e1127f4310c00e585f974>
 - xmr received: 0.480000000000

After this tx, total balance is: 0.480000000000


********************************************************************
Transaction: 12
********************************************************************

tx hash          : <a16f4658e736801ce1cc875e146127628810609b6297e362b86cd3c691d1a4d0>
public tx key    : <d61cb17aeb9931cce14763a657f9325685e8983db80f6b712f9f9d12d5543d15>
derived key      : <e8c971c052f32bd07dbfb7ba2d88b87133dea886e8672ed2c12bdcbac426c893>

Output no: 0, <347b6b50fef7e3dc3bc058e243b20aa4c012daed7514992363ac6b5c1c656719>, not mine key
Output no: 1, <52ecd4ac334a72dfcb10597f03aa93e2dcb1d6eea30c686cb1ac0d470d43df5c>, not mine key
Output no: 2, <372d28b918a96dc14690755690f61dad5534911708599c75eb2d5719d53d961b>, key_image: <e52aeab73a4eb94c448455ada910c1eb24d68d98a98232414bad43359d86d0c8>, mine key: 0.800000000000
Output no: 3, <5190e5a729dc0673961ce86f37b1671b796f5879bb6495a9341af7e1aa868219>, key_image: <cec87766f8a7d8c2313c62e7a1be9198ab51c65283166ef555e47048d6821ddd>, mine key: 4.000000000000
Output no: 4, <e0809bc7ed4e1d7c7869dfa035ba77d5f2412ce4baa01e7a6d841a5f1bb30a0f>, not mine key
Output no: 5, <467f05ed28d8bb0bc8421098c2088414a73318d7a77c92dfddd6825d7bf11851>, not mine key

Total xmr received: 4.800000000000

Input no: 0, <54653dae4730832d6b6a2ec02387990b4e8df16fecf22ccc21b7d5bc322b10d9>, not mine key image
Input no: 1, <97b698db50b743be28aa8cdefa5b2686543abe59db4de8b36940995a42e7cf9e>, not mine key image
Input no: 2, <f8c421fa85bbeb5fa253c000150b3d7d1430a679f91196f94ec0bbec6301e1ed>, not mine key image

Total xmr spend: 0.000000000000

Summary for tx: <a16f4658e736801ce1cc875e146127628810609b6297e362b86cd3c691d1a4d0>
 - xmr received: 4.800000000000

After this tx, total balance is: 5.280000000000


********************************************************************
Transaction: 13
********************************************************************

tx hash          : <1826bf767546ed1ebaebc22c34ca4f73e6f8b38efcb2ded79c9470d2625eadf9>
public tx key    : <e895ef3169a863e6f7ff80f744eca7aefcd5accc12c2b846d21d6c61088624f2>
derived key      : <79bda12aaffe27b48d3b2ab87b7d923bcbdbe2ff8eb41c21f6b2027e81cd5bd2>

Output no: 0, <127eeb7faefb0982be2b425cb1105d55f793177b93222a9e4044fb1f8a85f0c4>, not mine key
Output no: 1, <12e46d67e579a01676eda6938d3b85706d2f013f0dd7e05ef2bfd14d0f9492d1>, key_image: <96c9e5ad0882a1afdb8b0cda098cc80ab403eacac17597a3eddb76f0796aca6b>, mine key: 0.100000000000
Output no: 2, <0a63435b4380ed9a0d7081751dd9ce4a08f793cd67101d57a5634f23fb5d0e6d>, not mine key

Total xmr received: 0.100000000000

Input no: 0, <6c3eff3d76c78088fd2e80be9da13a7cbfe20e6f3482847f621e8194ff361e53>, mine key image: 0.400000000000

Total xmr spend: 0.400000000000

Summary for tx: <1826bf767546ed1ebaebc22c34ca4f73e6f8b38efcb2ded79c9470d2625eadf9>
- xmr spent: 0.300000000000 (includes tx fee: 0.020000000000)

After this tx, total balance is: 4.980000000000


********************************************************************
Transaction: 14
********************************************************************

tx hash          : <f81dd26e16c66a20e5609e6b19a849be7aaad3705ac7614db229a9d4f982bdaa>
public tx key    : <c76531bbe4424639b4fb53971ce003fe2d234de944ee30b72b14c00600cca36f>
derived key      : <7bef1e48e236995e54849cb2b5ed5fe34841f2161cc026af0330bb54c2539946>

Output no: 0, <411c35374bee9c91710e69c8ba6a22cc6383a0d3887d9d82cd3c17b50afcba4d>, not mine key
Output no: 1, <2297b3098b185622511e3e346d30d83c26cffca1231ccca122b6353729b9a54b>, key_image: <9c26520b06b9cdd483e2cc215e07757dc7f3aa358cc3d9a42c4fd594a76511a7>, mine key: 0.090000000000
Output no: 2, <f6e917103f388c856cbbab4870d59d74a09a58abe78b88f1f3b0de494a435e2e>, key_image: <28d4164fd321b34aeb2dcdab77bedff57ca0f9a299a808ea42d2ec74bb55c997>, mine key: 0.400000000000
Output no: 3, <dfadd91f5d66b530438e09f2710ae682035809b65c83a733cfe76db175ca2eb2>, key_image: <81d7b311ac8eed4f6f53d9f81c010f6dddf6a00441626947570f00dc6847a1e3>, mine key: 2.000000000000

Total xmr received: 2.490000000000

Input no: 0, <39754e9c8b152f84165c2dd84b92a0201b6311c223866cc920a229eb1556c87f>, not mine key image
Input no: 1, <d0547648701a0de906b8e2b0402e690c0702a0ea8344b8aa44c0b3eb3e806cd6>, not mine key image

Total xmr spend: 0.000000000000

Summary for tx: <f81dd26e16c66a20e5609e6b19a849be7aaad3705ac7614db229a9d4f982bdaa>
 - xmr received: 2.490000000000

After this tx, total balance is: 7.470000000000


********************************************************************
Transaction: 15
********************************************************************

tx hash          : <87f64738a14d25a8e4e1a6c2a78510895b4dc6ea1ab4f909ff1ccde8c6907f10>
public tx key    : <3d77a40759a0041ae0a74ddc1e6325e795018b89b72e2cb626c7189f5869afb0>
derived key      : <64c2095b03cf156946e60573890ed29a1977e64d7ba76a22790ae48e4d28b462>

Output no: 0, <b410e75ca1073bf7044ddc08afd0964284f3a797a739d0fcdb3065cf74850531>, key_image: <947fd3fd7a2ab3aef5933a198d8589c3e133b5f0f06dfb4ffe59cebcc4fa4fcc>, mine key: 0.030000000000
Output no: 1, <702e342e17e0fb117540620ad188b651a00e21a358bbc3287f50402f38b1fd8f>, not mine key
Output no: 2, <6697b8aecbd6f8899bcd730ce813e8f61c4e916acfb3719e93a60879d46d1ccd>, not mine key
Output no: 3, <888215fdd22659ea14e91439998af8315e5039be4edcf10f02c1c7941603c365>, key_image: <9eca6ec482fb8c3783fa683e0df46b51e97974d5583c98581193efe177c459b4>, mine key: 0.700000000000
Output no: 4, <4aa10a7b1f03cc508e1a96c7e597fd45e9e8e63980683f22f6c109f51a8d7295>, not mine key
Output no: 5, <186f07a480370f984e8155219d0a1061cf82f406e328f4e77e285b9378c9cfcf>, key_image: <c7e6133055f9970579329176508497fa867be3a978b09b9b4ee9251a0d9dc4ce>, mine key: 2.000000000000

Total xmr received: 2.730000000000

Input no: 0, <e4235e76bb0c45cb29dd633616f489193457262da9dcc23859a036a1378f9505>, mine key image: 0.080000000000
Input no: 1, <cec87766f8a7d8c2313c62e7a1be9198ab51c65283166ef555e47048d6821ddd>, mine key image: 4.000000000000

Total xmr spend: 4.080000000000

Summary for tx: <87f64738a14d25a8e4e1a6c2a78510895b4dc6ea1ab4f909ff1ccde8c6907f10>
- xmr spent: 1.350000000000 (includes tx fee: 0.010000000000)

After this tx, total balance is: 6.120000000000


********************************************************************
Transaction: 16
********************************************************************

tx hash          : <38b6935a9bc0385f5ddaf63582bb318a81125756a433fe2babab698f91438d20>
public tx key    : <6ab644a8e0f3e210383ba3e262c2361247aef1f99220e5256309125e5e7dc509>
derived key      : <964866fae38e34384b0862c81e225b19b44e96710bfe988de13755ee5ef46268>

Output no: 0, <4d872a49f29a3e13f5a2e82636a97ee6875b54246d3a948ebbcac9e86328e146>, not mine key
Output no: 1, <66f0e382f3b0f17b00b6f93c10ea9cb402498e90defe0fda070db39b97d062df>, not mine key
Output no: 2, <22a47a0140d3b86185546bc3124a7e53955f7efdaec21408f5affe269992f0e6>, not mine key
Output no: 3, <7f70425ca1ce756990e4c13e5219001d8a616a72ff763a2be5d8dd37a2a7ee8b>, not mine key
Output no: 4, <54c5c54001536874583ba7c7e6235f830ccf2d337757396e8f833bf7cf687623>, key_image: <d52ddc06f8fe26b3397e25dcb9c8d4733639be1f94386b5bfaa3eab41bad4b89>, mine key: 0.040000000000
Output no: 5, <4e3792a52c8c0a1313a2d9830f45d8538733016c5dd59e4ad21db768bfcebc17>, not mine key
Output no: 6, <6f8263d7133adb353eb6895130a0f5d1e0dab30d0f10cebc961d280d5e7fa77e>, key_image: <bbcd2dc76e3e392c4f2e02f7d289190a1827ee6f57550045998133a773432c2d>, mine key: 0.800000000000
Output no: 7, <7d88e5b61ffb023ba9fe0b8241c25f1a848b67955ecafaf7cd038a30f462095c>, not mine key

Total xmr received: 0.840000000000

Input no: 0, <81d7b311ac8eed4f6f53d9f81c010f6dddf6a00441626947570f00dc6847a1e3>, mine key image: 2.000000000000

Total xmr spend: 2.000000000000

Summary for tx: <38b6935a9bc0385f5ddaf63582bb318a81125756a433fe2babab698f91438d20>
- xmr spent: 1.160000000000 (includes tx fee: 0.036550000000)

After this tx, total balance is: 4.960000000000


********************************************************************
Transaction: 17
********************************************************************

tx hash          : <09d9e8eccf82b3d6811ed7005102caf1b605f325cf60ed372abeb4a67d956fff>
public tx key    : <4d571a359be8d781834ed8b5cbdc4786320f4d81b82fd9666ce3320b36969236>
derived key      : <06718e88aac4ea07fa7c20677df403b5ef71a8f8fe4255f5babc838be51437d0>

Output no: 0, <4cc297da762f016d0b4aa4057963121ed229b1818e001fae82dfbf06e5cd1688>, key_image: <4fdc7a804f54859cb42459e2b1ceb71a0a8f9b20ac26ede82494ec28e792212e>, mine key: 0.070000000000
Output no: 1, <a09d63055cea1e94273e6b782473cabd275a4ee28902b11ec79232f325f2cdb3>, key_image: <6032e53506b4f10813df2ed0ba1d12fe336f7a61c2dc339705cf53e89f43907f>, mine key: 0.300000000000
Output no: 2, <982aee5507e5d5d3399b6ca468f06570cdc8a2122cb46eeb2593ef799c4ed821>, not mine key

Total xmr received: 0.370000000000

Input no: 0, <9c26520b06b9cdd483e2cc215e07757dc7f3aa358cc3d9a42c4fd594a76511a7>, mine key image: 0.090000000000
Input no: 1, <9eca6ec482fb8c3783fa683e0df46b51e97974d5583c98581193efe177c459b4>, mine key image: 0.700000000000
Input no: 2, <947fd3fd7a2ab3aef5933a198d8589c3e133b5f0f06dfb4ffe59cebcc4fa4fcc>, mine key image: 0.030000000000
Input no: 3, <e52aeab73a4eb94c448455ada910c1eb24d68d98a98232414bad43359d86d0c8>, mine key image: 0.800000000000
Input no: 4, <96c9e5ad0882a1afdb8b0cda098cc80ab403eacac17597a3eddb76f0796aca6b>, mine key image: 0.100000000000
Input no: 5, <c7e6133055f9970579329176508497fa867be3a978b09b9b4ee9251a0d9dc4ce>, mine key image: 2.000000000000
Input no: 6, <d52ddc06f8fe26b3397e25dcb9c8d4733639be1f94386b5bfaa3eab41bad4b89>, mine key image: 0.040000000000
Input no: 7, <bbcd2dc76e3e392c4f2e02f7d289190a1827ee6f57550045998133a773432c2d>, mine key image: 0.800000000000

Total xmr spend: 4.560000000000

Summary for tx: <09d9e8eccf82b3d6811ed7005102caf1b605f325cf60ed372abeb4a67d956fff>
- xmr spent: 4.190000000000 (includes tx fee: 0.190000000000)

After this tx, total balance is: 0.770000000000


********************************************************************
Transaction: 18
********************************************************************

tx hash          : <83d682b3f1b57db488b1d2040a48c0db957e8b79f5e6142e1b018f41d4d9dc84>
public tx key    : <78170c5bf363fe3af3c8d88ae880170befcf294f5d679b2f42ab4e7b74c0e006>
derived key      : <4a38e8ac84d78ee3526426f8fdd84fd5911ff992069f8e44cc4b83fcdf63f545>

Output no: 0, <622e845b4ba6c2581c593342b7f0098fb99c89f90f4e51a14c88f086c963642c>, not mine key
Output no: 1, <6c1add6c9f01ab1ffeb9b33660aa58487ad86d53acdcaba01078152a0d118822>, not mine key

Total xmr received: 0.000000000000

Input no: 0, <28d4164fd321b34aeb2dcdab77bedff57ca0f9a299a808ea42d2ec74bb55c997>, mine key image: 0.400000000000
Input no: 1, <6032e53506b4f10813df2ed0ba1d12fe336f7a61c2dc339705cf53e89f43907f>, mine key image: 0.300000000000
Input no: 2, <4fdc7a804f54859cb42459e2b1ceb71a0a8f9b20ac26ede82494ec28e792212e>, mine key image: 0.070000000000

Total xmr spend: 0.770000000000

Summary for tx: <83d682b3f1b57db488b1d2040a48c0db957e8b79f5e6142e1b018f41d4d9dc84>
- xmr spent: 0.770000000000 (includes tx fee: 0.010000000000)

After this tx, total balance is: 0.000000000000

Final total balance: 0.000000000000

End of program.
```

The values agree with my  [manual record](https://github.com/moneroexamples/finding-mine-tx-ins-and-outs/blob/master/tx_manual_record.txt) and [simplewallet restoration log](https://github.com/moneroexamples/finding-mine-tx-ins-and-outs/blob/master/tx_restore_log.txt).


## Compile this example
The dependencies are same as those for Monero, so I assume Monero compiles
correctly. If so then to download and compile this example, the following
steps can be executed:

```bash
# download the source code
git clone https://github.com/moneroexamples/finding-mine-tx-ins-and-outs.git

# enter the downloaded sourced code folder
cd finding-mine-tx-ins-and-outs

# create the makefile
cmake .

# compile
make
```

After this, `tx_ins_and_outs` executable file should be present in access-blockchain-in-cpp
folder. How to use it, can be seen in the above example outputs.


## How can you help?

Constructive criticism, code and website edits are always good. They can be made through github.

Some Monero are also welcome:
```
48daf1rG3hE1Txapcsxh6WXNe9MLNKtu7W7tKTivtSoVLHErYzvdcpea2nSTgGkz66RFP4GKVAsTV14v6G3oddBTHfxP6tU
```
