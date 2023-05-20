#include "Variable.h"
#include "Constraint.h"

Constraint::Constraint():
    _rhs(1),
    _direction(EQUAL),
    _index(0),
    _lagrangean(0),
    _deleted(false),
    _covered(false)
{
    _variables.reserve(50);
}

Constraint::Constraint(float rhs, char dir = EQUAL, float ml = 0, int varCount = 50):
    _rhs(rhs),
    _direction(dir),
    _lagrangean(ml),
    _index(0),
    _deleted(false),
    _covered(false)
{
    _variables.reserve(varCount);
}

Constraint::Constraint(Constraint* r):
    _rhs(r->_rhs),
    _direction(r->_direction),
    _lagrangean(0),
    _index(r->_index),
    _deleted(r->_deleted),
    _covered(r->_covered)
{
    _variables.reserve(r->_variables.size());
}

Constraint* Constraint::CopyAndClean(Constraint* r) 
{
    if (r == NULL) {
        r = new Constraint(this);
    }
    else {
        r->_rhs = _rhs;
        r->_direction = _direction;
        r->_lagrangean = 0;
        r->_index = _index;
        r->_covered = _covered;
    }
    return r;
}


Constraint::~Constraint() 
{
    _variables.erase(_variables.begin(), _variables.end());
}


int Constraint::Degree() {
    VariableIterator it, itFim;
    ConstraintIterators(it, itFim);
    return distance(it,itFim);
}


bool Constraint::setDirection(char dir){
    if ( (dir >= LOWER_EQUAL) && ( dir <= GREATER_EQUAL ) ) {
        _direction = dir;
        return true;
    }
    return false;
}

void Constraint::setLagrangean(float ml) {
    _lagrangean = ml;
    if ( _direction  == LOWER_EQUAL ) {
        if ( _lagrangean > 0 )
            _lagrangean = 0;
    }

}


void Constraint::ConstraintIterators(VariableIterator& begin, VariableIterator& end) {
    begin = _variables.begin();
    end   = _variables.end();
}

void Constraint::SortVariablesByName()
{ 
    VariableIterator it;
    VariableIterator itFim;
    ConstraintIterators(it, itFim);
    sort(it, itFim, CompareNames <Variable*>());
}

void Constraint::CleanUpConstraint() {
    _variables.erase(_variables.begin(), _variables.end());
}



void Constraint::InsertVariable(Variable* var, float coef) {
    unsigned int i = _variables.size();
    unsigned int j = _variables.capacity();
    if (i == j) {
        i = (unsigned int)(i * 1.2);
        _variables.reserve(i);
    }
    _variables.push_back(var);
}

void Constraint::RemoveVariable(VariableIterator & it) {
    _variables.erase(it);
}

float Constraint::getIntercession(vector <Variable*>& sol)
{
    float count = 0;
    int solSize = sol.size();
    int i;

    VariableIterator it, itFim ;

    ConstraintIterators(it, itFim);

    for (i = 0; i < solSize; i++) {
        it = lower_bound(it, itFim, sol[i], CompareNames <Variable*>());
        if (it == itFim) return count;
        if ((*it)->_nome == sol[i]->_nome)
            count++;
    }

    return count;
}


float Constraint::getCoefficient(Variable* var)
{
    VariableIterator it, itFim;

    ConstraintIterators(it, itFim);
    if (binary_search(it, itFim, var, CompareNames <Variable*>()))
        return 1;
    return 0;
}



void Constraint::Print(FILE* fp) {
    int i = 1;
    VariableIterator it, itFim;

    ConstraintIterators(it, itFim);

    if (it == itFim) return;

    fprintf(fp, "x%d", (*it)->retNome());

    for (it++; it != itFim; it++) {
        if ((i % 12) == 0) fprintf(fp, "\n");
        fprintf(fp, " + x%d", (*it)->retNome());
        i++;
    }
    if (_direction == LOWER_EQUAL)
        fprintf(fp, "<");

    fprintf(fp, "= %f\n", _rhs);
    fflush(fp);
}


void Constraint::PrintBeasley() {
    int i = 0;
    VariableIterator it, itFim;
    
    ConstraintIterators(it, itFim);
    cout << distance(it,itFim);

    for (; it != itFim; it++) {
        if ((i % 12) == 0) cout << "\n";
        cout << (((*it)->_nome) + 1) << " ";
        i++;
    }
}