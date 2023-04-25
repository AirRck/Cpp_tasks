//Jakub Kaminski
#include <iostream>
#include <vector>
#include "Morfologia.h"

class BitmapaExt : public Bitmapa
{
    unsigned _sx, _sy, _sz;
    std::vector<std::vector<std::vector<char> > > _data;

public:
    BitmapaExt();
    BitmapaExt(unsigned sx, unsigned sy, unsigned sz);
    BitmapaExt(const Bitmapa& b);

    virtual unsigned sx() const ;
    virtual unsigned sy() const ;
    virtual unsigned sz() const ;

    virtual bool& operator()(unsigned x, unsigned y, unsigned z) ;
    virtual bool operator()(unsigned x, unsigned y, unsigned z) const ;

    friend std::ostream& operator<<(std::ostream& os, const BitmapaExt& b);
};


BitmapaExt::BitmapaExt():
    _sx(0), _sy(0), _sz(0), _data() {}

BitmapaExt::BitmapaExt(unsigned sx, unsigned sy, unsigned sz):
    _sx(sx), _sy(sy), _sz(sz),
    _data(_sx, std::vector<std::vector<char> >(_sy, std::vector<char>(_sz, 0)))
{
}

BitmapaExt::BitmapaExt(const Bitmapa& b):
    _sx(b.sx()), _sy(b.sy()), _sz(b.sz()), _data(_sx, std::vector<std::vector<char> >(_sy, std::vector<char>(_sz, 0)))
{
    for (unsigned x = 0; x < _sx; ++x) {
        for (unsigned y = 0; y < _sy; ++y) {
            for (unsigned z = 0; z < _sz; ++z) {
                _data[x][y][z] = b(x,y,z);
            }
        }
    }
}

unsigned BitmapaExt::sx() const
{
    return _sx;
}

unsigned BitmapaExt::sy() const
{
    return _sy;
}

unsigned BitmapaExt::sz() const
{
    return _sz;
}

bool& BitmapaExt::operator()(unsigned x, unsigned y, unsigned z)
{
    return reinterpret_cast<bool&>(_data[x][y][z]);
}

bool BitmapaExt::operator()(unsigned x, unsigned y, unsigned z) const
{
    return (bool)_data[x][y][z];
}

std::ostream& operator<<(std::ostream& os, const BitmapaExt& b)
{
    os << "{\n";
    for (unsigned x = 0; x < b._sx; ++x) {
        os << (x == 0 ? "" : ",\n") << " {\n";
        for (unsigned y = 0; y < b._sy; ++y) {
            os << (y == 0 ? "" : ",\n") << "  {";
            for (unsigned z = 0; z < b._sz; ++z) {
                os << (z == 0 ? "" : ",") << (bool)b._data[x][y][z];
            }
            os << "}";
        }
        os << "\n }";
    }
    return os << "\n}";
}

class PrzeksztalcenieZKopia : public Przeksztalcenie
{
protected:
    BitmapaExt o; // kopia oryginalnej
    Bitmapa* b; // docelowa
    unsigned sx, sy, sz;

    void init(Bitmapa& b) {
        this->o = BitmapaExt(b);
        this->b = &b;
        this->sx = b.sx();
        this->sy = b.sy();
        this->sz = b.sz();
    }
};

class Inwersja : public Przeksztalcenie
{
public:
    virtual void przeksztalc(Bitmapa& b)  {
        for (unsigned x = 0, sx = b.sx(); x < sx; ++x) {
            for (unsigned y = 0, sy = b.sy(); y < sy; ++y) {
                for (unsigned z = 0, sz = b.sz(); z < sz; ++z) {
                    b(x,y,z) = !b(x,y,z);
                }
            }
        }
    }
};

class Zerowanie : public Przeksztalcenie
{
public:
    virtual void przeksztalc(Bitmapa& b)  {
        for (unsigned x = 0, sx = b.sx(); x < sx; ++x) {
            for (unsigned y = 0, sy = b.sy(); y < sy; ++y) {
                for (unsigned z = 0, sz = b.sz(); z < sz; ++z) {
                    b(x,y,z) = false;
                }
            }
        }
    }
};

class ErozjaDylatacja : public PrzeksztalcenieZKopia
{
    bool hasOppsiteValue(unsigned x, unsigned y, unsigned z) {
        return x < sx && y < sy && z < sz && o(x,y,z) == !value;
    }

    void processMiddlePixel(unsigned x, unsigned y, unsigned z) {
        if (o(x,y,z) == value && (hasOppsiteValue(x-1,y,z) ||
                                  hasOppsiteValue(x,y-1,z) ||
                                  hasOppsiteValue(x,y,z-1) ||
                                  hasOppsiteValue(x+1,y,z) ||
                                  hasOppsiteValue(x,y+1,z) ||
                                  hasOppsiteValue(x,y,z+1))) {
            (*b)(x,y,z) = !value;
        }
    }

    bool value;

protected:
    void przeksztalc(Bitmapa& b, bool value) {
        init(b);
        this->value = value;
        for (unsigned x = 0; x < sx; ++x) {
            for (unsigned y = 0; y < sy; ++y) {
                for (unsigned z = 0; z < sz; ++z) {
                    processMiddlePixel(x, y, z);
                }
            }
        }
    }
};

class Erozja : public ErozjaDylatacja
{
public:
    virtual void przeksztalc(Bitmapa& b)  {
        ErozjaDylatacja::przeksztalc(b, true);
    }
};

class Dylatacja : public ErozjaDylatacja
{
public:
    virtual void przeksztalc(Bitmapa& b)  {
        ErozjaDylatacja::przeksztalc(b, false);
    }
};

class Usrednianie : public PrzeksztalcenieZKopia
{
    bool hasValue(unsigned x, unsigned y, unsigned z, bool value) {
        return x < sx && y < sy && z < sz && o(x,y,z) == value;
    }

    bool checkNeighbor(unsigned x, unsigned y, unsigned z, bool value) {
        return hasValue(x,y,z,value) && ++count >= 4;
    }

    void processPixel(unsigned x, unsigned y, unsigned z) {
        count = 0;
        bool v = o(x,y,z);
        if (checkNeighbor(x-1,y,z,!v) ||
            checkNeighbor(x,y-1,z,!v) ||
            checkNeighbor(x,y,z-1,!v) ||
            checkNeighbor(x+1,y,z,!v) ||
            checkNeighbor(x,y+1,z,!v) ||
            checkNeighbor(x,y,z+1,!v)) {
            (*b)(x,y,z) = !v;
        }
    }

    unsigned count;

public:
    virtual void przeksztalc(Bitmapa& b)  {
        init(b);
        for (unsigned x = 0; x < sx; ++x) {
            for (unsigned y = 0; y < sy; ++y) {
                for (unsigned z = 0; z < sz; ++z) {
                    processPixel(x, y, z);
                }
            }
        }
    }
};

class ZlozeniePrzeksztalcen : public Przeksztalcenie
{
    std::vector<Przeksztalcenie*> _data;
public:
    void dodajPrzeksztalcenie(Przeksztalcenie* p) {
        _data.push_back(p);
    }

    virtual void przeksztalc(Bitmapa& b)  {
        for (unsigned i = 0; i < _data.size(); ++i) {
            _data[i]->przeksztalc(b);
        }
    }
};
