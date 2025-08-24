#ifndef ALIASDATA_H
#define ALIASDATA_H

#include <QPair>

namespace Backend::Core
{

using PairInt = QPair<int, int>;
using PairDouble = QPair<double, double>;
using Matches = QList<PairInt>;
using ModalPairs = QList<QPair<int, double>>;
}

#endif // ALIASDATA_H
