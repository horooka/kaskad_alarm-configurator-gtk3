#ifndef ParamSelectH
#define ParamSelectH

#include "FastParamConst.h"
#include "BinDataStruct.h"
#include <vector>

// дескриптор одного параметра (дерево KLogic или служебный параметр)
struct SParamData {
    unsigned char DirType;
    unsigned int MaId;
    unsigned int CntId;
    unsigned int ParamId;
    unsigned char type;  // 0 = параметр дерева, 1 = служебный параметр
};

// тип значения для TPassport: 1 = дискретный, 2 = аналоговый (0 при ошибке)
int param_get_value_type(StMachins* m, const SParamData* d);

// перечислить параметры группы дерева KLogic
void params_list_from_tree_child(StMachins* m, unsigned char ma_id,
    unsigned short cnt_id, const StTreeChild* tree_child,
    std::vector<SParamData>& out);

// перечислить служебные параметры контроллера
void params_list_control(StMachins* m, unsigned char ma_id,
    unsigned short cnt_id, std::vector<SParamData>& out);

// заполнить TPassport по SParamData и типу значения
void param_data_to_passport(const SParamData* d, int value_type, TPassport* p);

// заполнить SParamData из TPassport (только KLogic/KLogicControl); true если успешно
bool passport_to_param_data(const TPassport* p, SParamData* d);

#endif
