       subroutine vedaloss
       implicit real*8(a-h,o-z)
       character*72 env_namefile
       character*72 ciar
       character  mate_name*20
       common /mainve/zr(29),ar(29),iaddress(29,2),tmate_type(29,6),
     #              coeff(14,2900),mate_name(29)
       ciar='db/loss1.vedasac'
c      env_namefile=ciar
c      ienv=solve_env_var(env_namefile,ciar)
       call LOSS_INIT(21,icode,ciar)
       if(icode.ne.7)write(6,*)'Lettura Tabella per Vedaloss: ok'
       if(icode.eq.7)write(6,*)'Errore Lettura Tabella'
c       call DELTA(ede,az,am,mate,EIN,ERE,ITER,
c     #                EIN1,ERE1,ITER1,eps,icode)
c       write(6,51)'DE = ',ede,' energia pre-spe = ',ein 

      return
      end
       subroutine LOSS_INIT(inp1,icode,ciar)
c      iniatialize the program loss to the default values
c      of the INDRA detector
       implicit real*8(a-h,o-z)
       character*72 ciar
       character*80  namefil
       character  mate_name*20
       common /mainve/zr(29),ar(29),iaddress(29,2),tmate_type(29,6),
     #              coeff(14,2900),mate_name(29)

       data RPP/623.61040/
 
       icode = 0
       iaddress(1,1) = 1
       iaddress(1,2) = 100
       iaddress(2,1) = 101
       iaddress(2,2) = 200
       iaddress(3,1) = 201
       iaddress(3,2) = 300
       iaddress(4,1) = 301
       iaddress(4,2) = 400
       iaddress(5,1) = 401
       iaddress(5,2) = 500
       iaddress(5,2) = 500
       iaddress(6,1) = 501
       iaddress(6,2) = 600
       iaddress(7,1) = 601
       iaddress(7,2) = 700
       iaddress(8,1) = 701
       iaddress(8,2) = 800
       iaddress(9,1) = 801
       iaddress(9,2) = 900
       iaddress(10,1) = 901
       iaddress(10,2) = 1000
       iaddress(11,1) = 1001
       iaddress(11,2) = 1100
       iaddress(12,1) = 1101
       iaddress(12,2) = 1200
       iaddress(13,1) = 1201
       iaddress(13,2) = 1300
       iaddress(14,1) = 1301
       iaddress(14,2) = 1400
       iaddress(15,1) = 1401
       iaddress(15,2) = 1500
       iaddress(16,1) = 1501
       iaddress(16,2) = 1600
       iaddress(17,1) = 1601
       iaddress(17,2) = 1700
       iaddress(18,1) = 1701
       iaddress(18,2) = 1800
       iaddress(19,1) = 1801
       iaddress(19,2) = 1900
       iaddress(20,1) = 1901
       iaddress(20,2) = 2000
       iaddress(21,1) = 2001
       iaddress(21,2) = 2100
       iaddress(22,1) = 2101
       iaddress(22,2) = 2200
       iaddress(23,1) = 2201
       iaddress(23,2) = 2300
       iaddress(24,1) = 2301
       iaddress(24,2) = 2400
       iaddress(25,1) = 2401
       iaddress(25,2) = 2500
       iaddress(26,1) = 2501
       iaddress(26,2) = 2600
       iaddress(27,1) = 2601
       iaddress(27,2) = 2700
       iaddress(28,1) = 2701
       iaddress(28,2) = 2800
       iaddress(29,1) = 2801
       iaddress(29,2) = 2900
 
       mate_name(1)    =  'Si'
       tmate_type(1,1) =    1
       tmate_type(1,2) =    1
       tmate_type(1,3) =  300.0 * 0.233          ! mg/cm^2   
       tmate_type(1,4) =    0

       mate_name(2)    =  'Mylar'
       tmate_type(2,1) =    2
       tmate_type(2,2) =    1
       tmate_type(2,3) =   2.5  * 0.1395         ! mg/cm^2   
       tmate_type(2,4) =    0

       mate_name(3)    =  'NE102'                 
       tmate_type(3,1) =    3
       tmate_type(3,2) =    1
       tmate_type(3,3) =   500. * 0.1032         ! mg/cm^2   
       tmate_type(3,4) =    0
 
       mate_name(4)    =  'Ni'                 
       tmate_type(4,1) =    4
       tmate_type(4,2) =    2
       tmate_type(4,3) =   0.2                   ! mg/cm^2   
       tmate_type(4,4) =    0
 
       mate_name(5)    =  'C3F8'                 
       tmate_type(5,1) =    5
       tmate_type(5,2) =   -1
       tmate_type(5,3) =    50. / 1.3333          !torr 
       tmate_type(5,4) =    50.                   !mm of gaz  
       tmate_type(5,5) =    188.                  !Molecula Weight
       tmate_type(5,6) =    19. + 273.15          !temperature (k)
       press = tmate_type(5,3)
       tmate_type(5,3) = (tmate_type(5,5) *
     # press * tmate_type(5,4))/
     #  (RPP*tmate_type(5,6))
 
       mate_name(6)    =  'C'                 
       tmate_type(6,1) =    6
       tmate_type(6,2) =    2
       tmate_type(6,3) =   0.20                 ! mg/cm^2   
       tmate_type(6,4) =    0
 
       mate_name(7)    =  'Ag'                 
       tmate_type(7,1) =    7
       tmate_type(7,2) =    2
       tmate_type(7,3) =   0.20                 ! mg/cm^2   
       tmate_type(7,4) =    0
 
       mate_name(8)    =  'Sn'                 
       tmate_type(8,1) =    8
       tmate_type(8,2) =    2
       tmate_type(8,3) =   0.20                   ! mg/cm^2   
       tmate_type(8,4) =    0
 
       mate_name(9)    =  'CsI'                 
       tmate_type(9,1) =    9
       tmate_type(9,2) =    3
       tmate_type(9,3) =   10.0 * 4510.0          ! mg/cm^2   
       tmate_type(9,4) =    0
 
       mate_name(10)    =  'Au'                 
       tmate_type(10,1) =   10
       tmate_type(10,2) =    2
       tmate_type(10,3) =   0.20                   ! mg/cm^2   
       tmate_type(10,4) =    0
 
       mate_name(11)    =  'U'                 
       tmate_type(11,1) =    11
       tmate_type(11,2) =    2
       tmate_type(11,3) =   0.20                   ! mg/cm^2   
       tmate_type(11,4) =    0
 
       mate_name(12)    =  'Air'                 
       tmate_type(12,1) =    12
       tmate_type(12,2) =    -1
       tmate_type(12,3) =    50. / 1.3333        !torr   
       tmate_type(12,4) =    100.                !mm of gaz
       tmate_type(12,5) =    28.848              !Molecula Weight
       tmate_type(12,6) =    19. + 273.15        !temperature (k)
       press = tmate_type(12,3)
       tmate_type(12,3) = (tmate_type(12,5) *
     # press * tmate_type(12,4))/
     #  (RPP*tmate_type(12,6))
 
       mate_name(13)    =  'Nb'                 
       tmate_type(13,1) =    13
       tmate_type(13,2) =    2
       tmate_type(13,3) =   0.20                 ! mg/cm^2   
       tmate_type(13,4) =    0
 
       mate_name(14)    =  'Ta'                 
       tmate_type(14,1) =    14
       tmate_type(14,2) =    2
       tmate_type(14,3) =   0.20                 ! mg/cm^2   
       tmate_type(14,4) =    0
 
       mate_name(15)    =  'V'                 
       tmate_type(15,1) =    15
       tmate_type(15,2) =    2
       tmate_type(15,3) =   0.20                 ! mg/cm^2   
       tmate_type(15,4) =    0
 
       mate_name(16)    =  'CF4'                 
       tmate_type(16,1) =    16
       tmate_type(16,2) =    -1
       tmate_type(16,3) =    50. / 1.3333        !torr   
       tmate_type(16,4) =    100.                !mm of gaz
       tmate_type(16,5) =    88.046              !Molecula Weight
       tmate_type(16,6) =    19. + 273.15        !temperature (k)
       press = tmate_type(16,3)
       tmate_type(16,3) = (tmate_type(16,5) *
     # press * tmate_type(16,4))/
     #  (RPP*tmate_type(16,6))
   
       mate_name(17)    =  'C4H10'                 
       tmate_type(17,1) =    17
       tmate_type(17,2) =    -1
       tmate_type(17,3) =    50. / 1.3333        !torr  
       tmate_type(17,4) =    100.                !mm of gaz
       tmate_type(17,5) =    58.0                !Molecula Weight
       tmate_type(17,6) =    19. + 273.15        !temperature (k)
       press = tmate_type(17,3)
       tmate_type(17,3) = (tmate_type(17,5) *
     # press * tmate_type(17,4))/
     #  (RPP*tmate_type(17,6))
 
       mate_name(18)    =  'Al'                 
       tmate_type(18,1) =    18
       tmate_type(18,2) =    1
       tmate_type(18,3) =   0.20 * 0.26989       ! mg/cm^2   
       tmate_type(18,4) =    0
 
       mate_name(19)    =  'Pb'                 
       tmate_type(19,1) =    19
       tmate_type(19,2) =    2
       tmate_type(19,3) =   0.20                 ! mg/cm^2   
       tmate_type(19,4) =    0
 
       mate_name(20)    =  'PbS'                 
       tmate_type(20,1) =    20
       tmate_type(20,2) =    2
       tmate_type(20,3) =   0.20                 ! mg/cm^2   
       tmate_type(20,4) =    0

       mate_name(21)    =  'KCl'                 
       tmate_type(21,1) =    21
       tmate_type(21,2) =    2
       tmate_type(21,3) =   0.20                ! mg/cm^2       densite 1.987
       tmate_type(21,4) =    0
 
       mate_name(22)    =  'Ge'                 
       tmate_type(22,1) =    22
       tmate_type(22,2) =    2
       tmate_type(22,3) =   0.20                ! mg/cm^2       densite 5.323  
       tmate_type(22,4) =    0
 
       mate_name(23)    =  'Ca'                
       tmate_type(23,1) =    23
       tmate_type(23,2) =    2
       tmate_type(23,3) =   0.20                ! mg/cm^2       densite 1.55
       tmate_type(23,4) =    0
 
       mate_name(24)    =  'Cu'                
       tmate_type(24,1) =    24
       tmate_type(24,2) =    2
       tmate_type(24,3) =   0.20                ! mg/cm^2       densite 8.96
       tmate_type(24,4) =    0

       mate_name(25)    =  'Ti'                
       tmate_type(25,1) =    25
       tmate_type(25,2) =    2
       tmate_type(25,3) =   0.20                ! mg/cm^2       densite 4.54
       tmate_type(25,4) =    0

       mate_name(26)    =  'Bi'                
       tmate_type(26,1) =    26
       tmate_type(26,2) =    2
       tmate_type(26,3) =   0.20                ! mg/cm^2       densite 9.747
       tmate_type(26,4) =    0

       mate_name(27)    =  'Mg'                
       tmate_type(27,1) =    27
       tmate_type(27,2) =    2
       tmate_type(27,3) =   0.20                ! mg/cm^2       densite 1.738
       tmate_type(27,4) =    0

       mate_name(28)    =  'Li'                
       tmate_type(28,1) =    28
       tmate_type(28,2) =    2
       tmate_type(28,3) =   0.20                ! mg/cm^2       densite 0.534
       tmate_type(28,4) =    0

       mate_name(29)    =  'Zn'                
       tmate_type(29,1) =    29
       tmate_type(29,2) =    2
       tmate_type(29,3) =   0.20                ! mg/cm^2       densite 7.133
       tmate_type(29,4) =    0


c  on ecrit le path de namefil en "dur" car ces routines peuvent etre utilisees
c  dans un environnement different de VEDA ou vedafil(1:long_path) n'existe pas

       namefil='db/loss1.vedasac'
cc       write(6,*)'Open energy loss_file: ',namefil
c       open(unit = inp1, file = namefil,
       open(unit = inp1, file = ciar,
     #     iostat=ios, status='old')
       if(ios.ne.0)then
        icode=7
        return
       endif

	do k=1,29
	read(inp1,*) zr(k),ar(k)
c	write(*,*) zr(K),ar(k)
         do j=iaddress(k,1), iaddress(k,2)
          read(inp1,*)(coeff(i,j),i=1,14)
         end do
	end do
       close(inp1)
       return

       end
