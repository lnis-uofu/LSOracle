  //object to hold STA configuration
STA sta_cfg;
ALICE_COMMAND( read_lib, "STA", "Reads standard cell library"){
    std::string filename = "";
    std::cout << "Enter liberty path: ";
    std::cin >> filename;

    const char *f = filename.c_str();

    sta::Sta *sta;
    sta::LibertyLibrary *myLib;
    sta::Corner corner("typycal", 0);

    myLib = sta->readLiberty(f, &corner, sta::MinMaxAll::min(), 0);

  }

  ALICE_COMMAND( read_netlist, "STA", "Reads mapped verilog"){
    std::string filename = "";
    std::cout << "Enter verilog path: ";
    std::cin >> filename;

    sta_cfg.set_netlist_path(filename);
  }

  ALICE_COMMAND( read_sdc, "STA", "Reads constraint file"){
    std::string filename = "";
    std::cout << "Enter sdc path: ";
    std::cin >> filename;

    sta_cfg.set_sdc_path(filename);
  }

  ALICE_COMMAND( run_slack, "STA", "Shows WNS and TNS"){

    sta_cfg.run_slack();
  }

  ALICE_COMMAND( report_critical_path, "STA", "Runs STA"){

    sta_cfg.run_report_timing();
  }
