#include "grafo.h"
#include "Constraint.h"
#include "RGPCut.h"


typedef map <Variable*, int, lessVariavel <Variable*>>::iterator GraphNodeIterator;

int contacortes = 0;

GraphNode::GraphNode() {
  _nivel = -1;
  _pai   = -1;
} 

GraphNode::GraphNode(const GraphNode &no) {
  _nivel  = no._nivel;
  _pai    = no._pai;
}


GraphNode::~GraphNode()
{ }


Grafo::Grafo(int size) {
    _nos.reserve(size);
    //_nosVariaveis.reserve(2*size);

    _arestas.reserve(size);
    _proximo = 0;
    _numeroNos = 0;
    _aumentou = false;
}

Grafo::Grafo() {
  _nos.reserve(40);
  //_nosVariaveis.reserve(80);
  _arestas.reserve(40);
  _proximo = 0;
  _numeroNos = 0;
  _aumentou = false;
}

Grafo::~Grafo() {
}

void Grafo::InsereNo(Variable *no) {
    _nos.push_back(no);
    _nosVariaveis[no] = _proximo;

    _infoNos.push_back(_NoBase);

    _proximo++;
    _numeroNos++;
    _aumentou = true;
}

void Grafo::InsereAresta(int no1, int no2) {
  
    if (_aumentou ) {
        _arestas.resize(_proximo + 3);
        _aumentou = false;
    }

    _arestas[no1].push_back(no2);
    _arestas[no2].push_back(no1);

}

void Grafo::InsereAresta(Variable *no1, Variable *no2) {
  
  if (_aumentou ) {
    _arestas.resize(_proximo + 3);
    _aumentou = false;
  }
  short int n1 = _nosVariaveis[no1];
  short int n2 = _nosVariaveis[no2];

  _arestas[n2].push_back(n1);
  _arestas[n1].push_back(n2);

}

int Grafo::Grau(int no) {
  return _arestas[no].size();
}

bool Grafo::TemAresta(int no1, int no2) {
  unsigned int i;
  for (i=0; i < _arestas[no1].size() ; i++) {
    if ( _arestas[no1][i] == no2 ) 
      return true;
  }
  
  return false;
  
}

/************* DFS *****************/

void Grafo::DFS(int no) {
  
  int indice;
  int prof;
  int noTeste;

  list <int> fila;  
  vector <bool> marca ( _numeroNos);

  for (indice=0; indice < _numeroNos; indice++) { 
     marca[indice] = false;
     _infoNos[indice]._pai   = -1;
     _infoNos[indice]._nivel = -1;
  }
  _dfsPath.erase(_dfsPath.begin(),_dfsPath.end());

  fila.push_front(no);
  _infoNos[no]._nivel = 0;
 
    while ( fila.size() != 0 ) {

        _dfsPath.push_back(no);
        marca[no] = true;
        fila.pop_front();
        prof = _infoNos[no]._nivel + 1;
        int tamanho = _arestas[no].size();

        for (indice=0; indice < tamanho; indice++) {
            noTeste = _arestas[no][indice];
            if ( ! marca[noTeste] ) {
                _infoNos[noTeste]._nivel = prof;
                _infoNos[noTeste]._pai = no;
                fila.push_front(noTeste);
            }
        } // for

        while ( fila.size() > 0 ) {
            no = fila.front();
            if ( marca[no] ) fila.pop_front();
            else break;		   
        }

    } // While 
}


/************* Clique ***************/

void Grafo::Clique(LagrangeanManager *mestre, int Minimo) {
 
    if (_aumentou ) {
        _arestas.resize(_proximo + 3);
        _aumentou = false;
    }

    vector <bool> marca( _numeroNos );
    RGPCut *corte;
    bool fim = false;

    unsigned int maiorGrau = 0;
    unsigned int grau;

    int candidata;
    int tamanho;
    int maior = 0;
    unsigned int no;
    unsigned int indice;

    GraphNodeIterator it;
    for (it = _nosVariaveis.begin(); it != _nosVariaveis.end(); it++) {
        no = (*it).second;
        grau = _arestas[no].size();
        if ( grau > 0 ) {
            marca[no] = false;
            if (  grau > maiorGrau ) {
	            maiorGrau = grau;
	            maior = no;
            }
        }
        else 
            marca[no] = true;
    } 
    fim = ( maiorGrau == 0 );
  
    while ( ! fim ) {

        corte = new RGPCut(); 
        corte->InsertVariable(_nos[maior],1);
        marca[maior] = true;
        tamanho = 1;

        //cout << "Escolhido: " << maior->retNome() << " " << indice << endl;

        for (indice = 0; indice < _arestas[maior].size(); indice++) {
            candidata = _arestas[maior][indice];
            VariableIterator rest;
            VariableIterator restFim;
            bool podeInserir = true;
      
            corte->ConstraintIterators(rest,restFim);
      
            for (; rest!= restFim; rest++) {
	            if ( ! TemAresta( _nosVariaveis[(*rest)], candidata  ) ) {
	                podeInserir = false;
	                break;
	            }	
            }  
            if ( podeInserir ) {
	            corte->InsertVariable(_nos[candidata],1);
	            marca[candidata] = true;
                tamanho++;
            }
        }

        if (tamanho >= Minimo ) {
            mestre->InsertCut(corte);
            contacortes++;    
            //corte->Imprime();
        }
        else delete corte;

        fim = true;
        for (it=_nosVariaveis.begin(); it != _nosVariaveis.end(); it++) {
            no = (*it).second;
            if ( ! marca[no] ) {
	            maiorGrau = _arestas[no].size();
	            maior = no;
	            fim = false;
	            break;
            }
        } 
    
    } /* while (! fim ) */
}


/************* Ciclo Impar **************/


void Grafo::CicloImpar(LagrangeanManager *mestre) {

    if (_aumentou ) {
        _arestas.resize(_proximo+3);
        _aumentou = false;
    }

    int no,noTeste;
    int indice;
    int grau;
    RGPCut *corte;


    no = 0;
    DFS(no);
    grau = _arestas[no].size();
    for (indice = 0; indice < grau; indice++) {
        noTeste = _arestas[no][indice];
        if ( ( _infoNos[noTeste]._nivel > 3 ) && (_infoNos[noTeste]._nivel % 2 == 0) && (noTeste > no) ) {
            corte = new RGPCut( (float) ((int) (_infoNos[noTeste]._nivel / 2)) );
            while ( noTeste >= 0 ) {

	            corte->InsertVariable(_nos[noTeste],1);
	            noTeste = _infoNos[noTeste]._pai;
            }
            mestre->InsertCut(corte);
            contacortes++;
            break;
        }
    }
   
}

void Grafo::Imprime() {

    if (_aumentou ) {
        _arestas.resize(_proximo + 3);
        _aumentou = false;
    }

    cout << _numeroNos << endl;
    unsigned int i = 0;
    unsigned int iFim = _numeroNos; 
    vector <int>::iterator itVizinhos, itVizinhosFim;

    for (; i < iFim; i++) {
        cout << _nos[i]->retNome() << " --- ";
        itVizinhos    = _arestas[i].begin();
        itVizinhosFim = _arestas[i].end();
        for (; itVizinhos != itVizinhosFim; itVizinhos++) {
            cout << _nos[*itVizinhos]->retNome() << " ";
        }
        cout << _arestas[i].size() << " " << i << " " << iFim << endl;
    }

}

void Grafo::ImprimeNos() {
    for (unsigned int k = 0 ; k < (unsigned int)_numeroNos; k++ ) 
        cout << _nos[k]->retNome() << " " << _infoNos[k]._pai << " ";
    cout << endl;
}

void Grafo::ImprimeDFS() {
  Imprime();
  int no;
  list <int>::const_iterator it;
  
  for (it = _dfsPath.begin(); it != _dfsPath.end(); it++ ) {
    no = *it;
    cout << _nos[no]->retNome() << " " << no << " " << _infoNos[no]._nivel << " " << _infoNos[no]._pai << endl;
  }


}
