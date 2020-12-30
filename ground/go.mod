module github.com/dictor/hamstrone_ground

go 1.15

require (
	github.com/dictor/echologrus v1.1.5
	github.com/dictor/hamstrone_ground/hamstertongue v0.0.0
	github.com/dictor/wswrapper v1.2.2
	github.com/jacobsa/go-serial v0.0.0-20180131005756-15cf729a72d4
	github.com/labstack/echo/v4 v4.1.17
	github.com/labstack/gommon v0.3.0
	github.com/sirupsen/logrus v1.7.0
)

replace github.com/dictor/hamstrone_ground/hamstertongue v0.0.0 => ./hamstertongue
