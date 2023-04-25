#ifndef __MORFOLOGIA_H__
#define __MORFOLOGIA_H__

class Bitmapa
{
public:
  virtual unsigned sx() const = 0;
  virtual unsigned sy() const = 0;
  virtual unsigned sz() const = 0;

  virtual bool& operator()(unsigned x, unsigned y, unsigned z) = 0;
  virtual bool operator()(unsigned x, unsigned y, unsigned z) const = 0;

  virtual ~Bitmapa(){}
};

class Przeksztalcenie
{
public:
  virtual void przeksztalc( Bitmapa& ) = 0;
  virtual ~Przeksztalcenie() {}
};

#endif
