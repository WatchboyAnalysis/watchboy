standard: main

main: gate_builder corrupt_worker gate_builder_addon gate_builder_organize charge_balance organize

organize: organize.cpp
	g++ `root-config --cflags --libs` -std=c++0x organize.cpp -o organize
	chmod ug+rwx organize

charge_balance: charge_balance.cpp
	g++ `root-config --cflags --libs` -std=c++0x charge_balance.cpp -o charge_balance
	chmod ug+rwx charge_balance

gate_builder_organize: gate_builder_organize.cpp
	g++ `root-config --cflags --libs` -std=c++0x gate_builder_organize.cpp -o gate_builder_organize
	chmod ug+rwx gate_builder_organize

gate_builder_addon: gate_builder_addon.cpp
	g++ `root-config --cflags --libs` -std=c++0x gate_builder_addon.cpp -o gate_builder_addon
	chmod ug+rwx gate_builder_addon

gate_builder: gate_builder.cpp
	g++ `root-config --cflags --libs` -std=c++0x gate_builder.cpp -o gate_builder
	chmod ug+rwx gate_builder

corrupt_worker: corruption_test_worker.cpp
	g++ `root-config --cflags --libs` -std=c++0x corruption_test_worker.cpp -o corrupt_worker
	chmod ug+rwx corrupt_worker

clean:
	rm -f corrupt_worker gate_builder gate_builder_addon gate_builder_organize charge_balance
