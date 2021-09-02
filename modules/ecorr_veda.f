      subroutine ecorr_veda(eingresso,zpr,apr,atar,eout,elost,mate,
     c    thick,idir,icod,pressione)
c

c con idir=1 data eingresso (energia di ingresso nel rivelatore) calcola l'energia residua eout dopo il passaggio nel rivelatore di spessore thick in micron; elost e' l'energia persa nel  rivelatore

c con idir=2 data eingresso (energia lasciata nel rivelatore al silicio) calcola l'energia eout prima dell'attraversamento di uno spessore morto thick in micron; elost e' l'energia lasciata nello spessore morto


      real*8 amved,azved,enved,einved,eps,spesved,amasr,eresi
      real*8 coeff(14,2900),tmate_type(29,6),zr(29),ar(29)       
      dimension density(29)
       common /mainve/zr,ar,iaddress(29,2),tmate_type,
     #              coeff,mate_name(29)
      character  mate_name*20
       data density /0.233, 0.1395, 0.1032, 0.8902, 1.3333, 0.190,
     # 1.05, 0.713, 0.451, 1.93, 1.895, 1.3333, 0.857, 1.6654, 0.611, 
     # 1.3333, 1.3333, 0.26989, 1.135, 0.75, 0.1987, 0.5323, 0.155,
     # 0.896, 0.454, 0.9747, 0.1738, 0.0534, 0.7133/

       data RPP/623.61040/

c       data density /0.233, 0.1395, 0.1032, 0.8902, 1.3333, 0.190,
c     # 1.05, 0.713, 0.451, 1.93,1.895, 0.270, 0.0, 0.0, 0.0, 0.0, 0.0,
c     # 0.0, 0.0, 0.0/
      eps=0.001               ! max diff. tra decalc e dein
c      amasr=0.
      amasr=atar
      if(tmate_type(mate,2).eq.1)spesved=thick
      if(tmate_type(mate,2).eq.2)spesved=thick*density(mate)
      if(tmate_type(mate,2).eq.3)spesved=thick/10000   ! ????
c      if(tmate_type(mate,2).eq.-1)spesved=thick/1000*density(mate)**2
      if(tmate_type(mate,2).eq.-1) then
         tmate_type(mate,4)=thick/1000.
         spesved=pressione !mbar
         tmate_type(mate,6)=20.+273.15!temp in kelvin (fissa)
      endif

      azved=nint(zpr)
      amved=apr
      enved=eingresso
      call SET_THICKNESS(spesved,amasr,mate)
      call PARAM(enved,azved,amved,mate,einved,eresi,idir,icode)
      eout=eresi
      elost=einved
      icod=icode
      return
      end
       subroutine SET_THICKNESS(thick,amasr,id)
c	New argument amasr (01/09/97)
c      set the current thickness values
       implicit real*8(a-h,o-z)
       character  mate_name*20
       dimension density(29)
       common /mainve/zr(29),ar(29),iaddress(29,2),tmate_type(29,6),
     #              coeff(14,2900),mate_name(29)
       data density /0.233, 0.1395, 0.1032, 0.8902, 1.3333, 0.190,
     # 1.05, 0.713, 0.451, 1.93, 1.895, 1.3333, 0.857, 1.6654, 0.611, 
     # 1.3333, 1.3333, 0.26989, 1.135, 0.75, 0.1987, 0.5323, 0.155,
     # 0.896, 0.454, 0.9747, 0.1738, 0.0534, 0.7133/


       data RPP/623.61040/
       
       if(amasr.eq.0.0)then 
        amas=ar(id) 
       else
        amas = amasr
       endif      
       if(tmate_type(id,2).eq.1) tmate_type(id,3)=thick*density(id)*
     # ar(id)/amas
       if(tmate_type(id,2).eq.-1) tmate_type(id,3)=(tmate_type(id,5)
     #  *(thick/density(id))*tmate_type(id,4))/
     #  (RPP*tmate_type(id,6))
       if(tmate_type(id,2).eq.2) tmate_type(id,3)=thick*ar(id)/amas
       if(tmate_type(id,2).eq.3) tmate_type(id,3)=thick*density(id)*
     # 10000.0*ar(id)/amas
c       write(6,*)tmate_type(id,3)
       return
       end
 
       subroutine GET_THICKNESS(thick,amasr,id)
c	New argument amasr (01/09/97)
c      Get the current thickness values or pression
       implicit real*8(a-h,o-z)
       character  mate_name*20
       dimension density(29)
       common /mainve/zr(29),ar(29),iaddress(29,2),tmate_type(29,6),
     #              coeff(14,2900),mate_name(29)
       data density /0.233, 0.1395, 0.1032, 0.8902, 1.3333, 0.190,
     # 1.05, 0.713, 0.451, 1.93, 1.895, 1.3333, 0.857, 1.6654, 0.611, 
     # 1.3333, 1.3333, 0.26989, 1.135, 0.75, 0.1987, 0.5323, 0.155,
     # 0.896, 0.454, 0.9747, 0.1738, 0.0534, 0.7133/

       data RPP/623.61040/
 
	if(amasr.eq.0.0) amasr=ar(id)       
       if(tmate_type(id,2).eq.1) thick=tmate_type(id,3)*amasr/
     # ar(id)/density(id)
       if(tmate_type(id,2).eq.-1) thick=(RPP*tmate_type(id,6)*
     # tmate_type(id,3))/(tmate_type(id,5)*tmate_type(id,4))*
     # density(id)
       if(tmate_type(id,2).eq.2) thick=tmate_type(id,3)*amasr/ar(id)
       if(tmate_type(id,2).eq.3) thick=tmate_type(id,3)*amasr/
     # ar(id)/density(id)/10000.0 

       return
       end
 
c       subroutine SET_TEMP_GAZ(temp,id)
c       implicit real*8 (a-h,o-z)
c       character mate_name*20
c       common /mainve/coeff(14,2000),iaddress(20,2),tmate_type(20,6),
c     #              mate_name(20),zr(20),ar(20)
 
c       if(tmate_type(id,2).eq.-1)then
c        tmate_type(id,6) = temp + 273.15
c        write(*,10)mate_name(id),temp
c       endif
c10     format('Gaz ',a5,'temperature set to ',f5.1,' C')
c       return
c       end
 

*****************************************************************************
*      INDRAloss library      (modifie:  R.Dayras & E.de FILIPPO 24/5/96)   *
*                             (last modified:  R. Dayras      1/9/97        *
*                             (old version in vedasac.f_old)                *
*****************************************************************************
      
       subroutine DELTA(ede,az,am,mate,EIN,ERE,ITER,
     #                EIN1,ERE1,ITER1,eps,icode)
c      calculate the incident energy for a given 
c      material knowing the DeltaE
      
       implicit real*8(a-h,o-z) 
       parameter (idirect=1, inverse=2, jmax=100)  
       character  mate_name*20
       common /mainve/zr(29),ar(29),iaddress(29,2),tmate_type(29,6),
     #              coeff(14,2900),mate_name(29)      


       
       integer*4 nbloc,nnevt,nevt,nevtot,ngene,numrun
        
       Common /STATEVT/nbloc,nnevt,nevt,nevtot,ngene,numrun
       Common /toto/ii
       
       icode = 0
       EIN=0
       ERE=0
       EIN1=0
       ERE1=0   
       iter = 1
       eres = 0.0001 * am 
       idr = inverse 
       call PARAM(eres,az,am,mate,ed,ei,idr,icode)
cc       write(6,*)'pluto',eres,az,am,mate,ed,ei,idr,icode
      if(icode.eq.3)then
        return
       endif
c       if(ede.gt.ed)then 
c        icode=1
c        return 
c       endif
       e1 = ed
       if(az.eq.1)then 
         e2 = 400. * am
       elseif(az.ge.2)then 
         e2 = 200. * am 
       endif
       idr   = idirect
       en = abs(e2 - e1) /2.0 + e1
c       write(6,*)'f',en,az,am,mate,dcalc,eres,idr,icode
       call FAST_PARAM(en,az,am,mate,dcalc,eres,idr,icode)
       do j=1,jmax
         diff = dcalc - ede
         difa = abs(diff)
         if(difa.lt.eps)then
          goto 10
         elseif(diff.lt.0.)then
          e2 = en
         elseif(diff.gt.0.)then
          e1 = en
         endif
         en = abs(e2 - e1) /2.0 + e1
         iter = iter + 1
c        param is called faster without charging parameters
c        that are always the same in the iteration procedure
         call FAST_PARAM(en,az,am,mate,dcalc,eres,idr,icode) !entryparam
c       write(6,*)'fdj',j,en,az,am,mate,dcalc,eres,idr,icode
        enddo
5      continue 
       icode=2
       return
10     continue 
       EIN = en 
       ERE  = en - ede
       
c       Recherche de la deuxieme solution 
       e1 = ed
       e2 = ein 
       iter1=0
       idr   = idirect
       en = abs(e2 - e1) /2.0 + e1
       call FAST_PARAM(en,az,am,mate,dcalc,eres,idr,icode)
       do j=1,jmax
         diff = dcalc - ede
         difa = abs(diff)
         if(difa.lt.eps)then
          goto 20
         elseif(diff.gt.0.)then
          e2 = en
         elseif(diff.lt.0.)then
          e1 = en
         endif
         en = abs(e2 - e1) /2.0 + e1
         iter1 = iter1 + 1
c        param is called faster without charging parameters
c        that are always the same in the iteration procedure
         call FAST_PARAM(en,az,am,mate,dcalc,eres,idr,icode) !entryparam
        enddo
6      continue 
       icode=0
       return
20     continue 
       EIN1 = en 
       ERE1  = en - ede
       return 
       end
      
       subroutine PARAM(einp,az,am,id,EL,ER,idirec,ICODE)
      
c      **************************************************
c         calculation of energy loss in a given 
c              material by polynomial fit
c      ************************************************** 
c      input   einp,az,am,mate,direc [idirect,inverse]
c      output  EL (energy loss) ER (Res. energy | Inc. energy)
c      call    function eloss
      
       parameter (istout=6) 
       implicit real*8(a-h,o-z) 
       character mate_name*20
        common /mainve/zr(29),ar(29),iaddress(29,2),tmate_type(29,6),
     #              coeff(14,2900),mate_name(29)      
      

       common/los/vect(15),perc,ratio
        
       integer*4 nbloc,nnevt,nevt,nevtot,ngene,numrun
        
       Common /STATEVT/nbloc,nnevt,nevt,nevtot,ngene,numrun
       Common /toto/ii
       
       PERC = 0.02
       icode = 0     
       vect(15) = tmate_type(id,3) 
       call GET_PARAM(id,amasr,az,icode) 
       if(icode.ne.0)then
        return
       endif
    
c      fast entry (for iterative procedures)
c       write(6,*)'prima'
       ENTRY FAST_PARAM(einp,az,am,id,EL,ER,idirec,ICODE)
c       write(6,*)'dopo'
       ER = eloss(einp,am,idirec)
       
       if (idirec.eq.1)then
         EL = einp - er
c         if (EL.lt.0.005)then !com
c          EL=0.0 !com
c          ER=einp !com
c         endif   !com
       else
         EL = ER - einp
       end if  
       
       return
c      end param
       end   !subroutine param
      
      
       subroutine GET_PARAM(id,amasr,az,ICODE)
c	New argument amasr (01/09/97)
       implicit real*8(a-h,o-z)
       character mate_name*20
        common /mainve/zr(29),ar(29),iaddress(29,2),tmate_type(29,6),
     #              coeff(14,2900),mate_name(29)      
       common/los/vect(15),perc,ratio
       common/estrapo/adm,adn,arm
       index = id
	if(amasr.eq.0.0) amasr=ar(id)
       ind = iaddress(index,1) + az - 1
       if ((az.eq.coeff(1,ind)).and.
     #     (ind.le.iaddress(index,2))) then
	ratio=amasr/ar(id) 
        do j=1,14
          vect(j) = coeff(j,ind)
        end do
       else
        icode = 3
        return    
       end if 
       
       
       x1 = log(0.1) 
       x2 = log(0.2) 
       ran= 0.0
       do j= 2,6
         ran = ran + vect(j+2) * x2**(j-1) 
       end do
       ran = ran + vect(3) 
       y2 = ran 
       ran= 0.0
       do j= 2,6
         ran = ran + vect(j+2) * x1**(j-1) 
       end do
       ran = ran + vect(3) 
       y1 = ran 
       adm=(y2-y1)/(x2-x1)
       adn=(y1-adm*x1)
       arm = y1
       return
       end
       
       function ELOSS(einp,am,idirec)
c      ***********************************************
c      called by PARAM
c      ***********************************************
       implicit real*8(a-h,o-z)
       common/los/vect(15),perc,ratio
       common/estrapo/adm,adn,arm
       
       integer*4 nbloc,nnevt,nevt,nevtot,ngene,numrun
        
       Common /STATEVT/nbloc,nnevt,nevt,nevtot,ngene,numrun
       
       if(einp.le.0.)then
         if (idirec.eq.1) then
           eloss = 0.
         else
           eloss = -1.
         end if
         goto 100
       end if

       eps   =  einp/am
       dleps =  dlog(eps)
       riso  =  dlog(am/vect(2))
c vect(2) e' la massa della part
c       write(6,*)'entro',eps,vect(2),riso
       if(eps.lt.0.1)then 
        ran = adm*dleps+adn
       else
        ran= 0.0
        do j= 2,6
         ran = ran + vect(j+2) * dleps**(j-1) 
c         write(6,*)ran
        end do
        ran = ran + vect(3)
c        write(6,*)'ranf',ran
       endif 
       ran=ran+riso       
c vect=coeff di perdita di energia
       range = dexp(ran)
c      if(idirec.eq.1)type*,eps,range    !test range
       if(idirec.eq.1) then       
        range = range - vect(15)
        if (range.le.0.005) then
          eloss = 0.
          goto 100
        end if
       else
        range = range + vect(15)
       end if
c vect(15)=mg/cm^2 del riv
       ranx = dlog(range)
       ranx1 = ranx - riso

       if(ranx1.lt.arm)then 
        depsx = (ranx1-adn)/adm
       else
       depsx = 0.0
       do j=2,6
         depsx = depsx+ vect(j+8)*ranx1**(j-1)
       end do
       depsx = depsx + vect(9)
       endif

       eps1 = depsx + dlog(1-perc)     
       eps2 = depsx + dlog(1+perc)
       rap = dlog ((1+perc)/(1-perc))
   
       rn1=0.0
       if( dexp(eps1).lt.0.1)then 
        rn1 = adm*eps1+adn
       else
       do j= 1,6
        rn1 = rn1 + vect(j+2) * eps1**(j-1) 
       end do
       endif
       rn2=0.0
       if( dexp(eps2).lt.0.1)then 
        rn2 = adm*eps2+adn
       else
       do j= 1,6
        rn2 = rn2 + vect(j+2) * eps2**(j-1) 
       end do
       endif
       epres = eps1 + (rap/(rn2-rn1)) * (ranx1-rn1)
       epres = dexp(epres)

       eloss = am*epres 
100    continue
       return
c      end eloss
       end    !function eloss
       

       subroutine RANGE(einc,az,am,id,amasr,RANG,ICODE)
c	New argument amasr (01/09/97)
c      calculate the range for a given energy
c      and mass of the incident particle  

       implicit real*8(a-h,o-z)
       common/los/vect(15),perc,ratio
       common/estrapo/adm,adn,arm

       icode = 0 
       call GET_PARAM(id,amasr,az,icode)
       if(icode.ne.0)then
        return
       endif 
       if(einc.le.0.)then
        rang = 0.
        return
       endif 
       eps   =  einc/am
       dleps =  dlog(eps)
       riso  =  dlog(am/vect(2))

       if(eps.lt.0.1)then 
        ran = adm*dleps+adn
       else
        ran= 0.0
        do j= 2,6
         ran = ran + vect(j+2) * dleps**(j-1) 
        end do
        ran = ran + vect(3)
       endif 
       ran=ran+riso
       RANG = ratio*dexp(ran)
       return    
       end

       Subroutine Set_Gaz_Param(temp,epes,id)
       implicit real*8 (a-h,o-z)
       character  mate_name*20
        common /mainve/zr(29),ar(29),iaddress(29,2),tmate_type(29,6),
     #              coeff(14,2900),mate_name(29)      


       if(tmate_type(id,2).eq.-1)then   !it is a gaz
        tmate_type(id,4) = epes
        tmate_type(id,6) = temp + 273.15
       endif
       return
       end


       subroutine ERROR_CODE(iw,icode)
       character*79 code(0:10)

       if(icode.ge.10)icode=10
       code(0) = ' OK'
       code(1) = ' Error: The particle stops in the detector'
       code(2) = ' Error: Too many iterations or energy out of range'
       code(3) = ' Error module PARAM: Z not found '
       code(5) = ' Error open input file'
       code(6) = ' Error read from input-data file'
       code(7) = ' Error open data file'
       code(10)= ' *** Error *** '
       write(iw,'(a)')code(icode)
       return
       end


       subroutine EXITP
c      exit the program
       stop
       end
****************** end indraloss library *************************


      subroutine de_vedaloss(z,a,atar,de,spess,e,mate,pressione)
c dato il delta e nel silicio sie(k) calcola l'energia etot prima del silicio di spessore spesved 
c RICORDARSI CHE zzz DEVE ESSERE nint(z)
c npart in common
      real*8 amved,azved,enved,einved,eps,spesved,amasr,eran
      real*8 coeff(14,2900),tmate_type(29,6),zr(29),ar(29)
       common /mainve/zr,ar,iaddress(29,2),tmate_type,
     #              coeff,mate_name(29)
      character  mate_name*20
      dimension density(29)
      real*8 ere,ein1,ere1
       data density /0.233, 0.1395, 0.1032, 0.8902, 1.3333, 0.190,
     # 1.05, 0.713, 0.451, 1.93, 1.895, 1.3333, 0.857, 1.6654, 0.611, 
     # 1.3333, 1.3333, 0.26989, 1.135, 0.75, 0.1987, 0.5323, 0.155,
     # 0.896, 0.454, 0.9747, 0.1738, 0.0534, 0.7133/

       data RPP/623.61040/

c      data density /0.233, 0.1395, 0.1032, 0.8902, 1.3333, 0.190,
c     # 1.05, 0.713, 0.451, 1.93,1.895, 0.270, 0.0, 0.0, 0.0, 0.0, 0.0,
c     # 0.0, 0.0, 0.0/

      eps=0.001               ! max diff. tra decalc e dein
c      amasr=0.
      amasr=atar
      if(tmate_type(mate,2).eq.1)spesved=spess
      if(tmate_type(mate,2).eq.2)spesved=spess*density(mate)
      if(tmate_type(mate,2).eq.3)spesved=spess/10000
      if(tmate_type(mate,2).eq.-1)then
         spesved=pressione !mbar
         tmate_type(mate,4)=spess/1000.
         tmate_type(mate,6)=20.+273.15!temp in kelvin (fissa)
      endif
      azved=nint(z)
      call SET_THICKNESS(spesved,amasr,mate)
      enved=de
      amved=a
      call DELTA(enved,azved,amved,mate,einved,ERE,ITER,
     #                      EIN1,ERE1,ITER1,eps,icode)
      e=einved
      return
      end


 

 
 
 
 
