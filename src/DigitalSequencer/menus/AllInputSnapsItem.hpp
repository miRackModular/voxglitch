#include "AllInputSnapsValueItem.hpp"

struct AllInputSnapsItem : MenuItem {
  DigitalSequencer *module;

  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    for (unsigned int i=0; i < NUMBER_OF_SNAP_DIVISIONS; i++)
    {
      dseq::AllInputSnapsValueItem *all_input_snaps_value_item = createMenuItem<dseq::AllInputSnapsValueItem>(module->snap_division_names[i]);
      all_input_snaps_value_item->module = module;
      all_input_snaps_value_item->snap_division_index = i;
      menu->addChild(all_input_snaps_value_item);
    }

    return menu;
  }
};