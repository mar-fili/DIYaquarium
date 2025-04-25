package main

import (
	"fmt"
	"io"
	"net"
	"os"
	"time"

	"gioui.org/app"
	"gioui.org/layout"
	"gioui.org/op"
	"gioui.org/unit"
	"gioui.org/widget"
	"gioui.org/widget/material"
)

func main() {
	var response string

	go func() {
		w := new(app.Window)
		w.Option(app.Title("ESP Control"))
		w.Option(app.Size(unit.Dp(400), unit.Dp(600)))

		var ops op.Ops
		th := material.NewTheme()

		// Dodane suwaki i przycisk
		var sliders [4]widget.Float
		var sendButton widget.Clickable
		var sendButton1 widget.Clickable
		var timeEditor widget.Editor
		timeEditor.Submit = true

		for {
			e := w.Event()
			switch evt := e.(type) {
			case app.FrameEvent:
				gtx := app.NewContext(&ops, evt)

				layout.Center.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
					return layout.Flex{
						Axis:    layout.Vertical,
						Spacing: layout.SpaceEvenly,
					}.Layout(gtx,

						// Suwaki
						layout.Rigid(func(gtx layout.Context) layout.Dimensions {
							return renderSlider(gtx, th, &sliders[0], "Biały 1")
						}),
						layout.Rigid(func(gtx layout.Context) layout.Dimensions {
							return renderSlider(gtx, th, &sliders[1], "Biały 2")
						}),
						layout.Rigid(func(gtx layout.Context) layout.Dimensions {
							return renderSlider(gtx, th, &sliders[2], "Niebieski")
						}),
						layout.Rigid(func(gtx layout.Context) layout.Dimensions {
							return renderSlider(gtx, th, &sliders[3], "Czerwony")
						}),

						// Pole tekstowe na godziny
						layout.Rigid(func(gtx layout.Context) layout.Dimensions {
							return layout.Inset{Top: 8, Bottom: 8, Left: 16, Right: 16}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
								return material.Editor(th, &timeEditor, "hhmm-hhmm").Layout(gtx)
							})
						}),

						layout.Rigid(func(gtx layout.Context) layout.Dimensions {
							btn := material.Button(th, &sendButton, "Wyślij")
							if sendButton.Clicked(gtx) {
								go func() {
									var cmd string
									if timeEditor.Text() != "" {
										cmd = "Schedule "
									} else {
										cmd = "Set "
									}
									for i := range 4 {
										cmd += fmt.Sprintf("%03d", int(sliders[i].Value*100))
										cmd += " "
									}
									if timeEditor.Text() != "" {
										cmd += timeEditor.Text()
									}
									resp, err := sendRawCommand(cmd)
									if err != nil {
										response = "Błąd: " + err.Error()
									} else {
										response = resp
									}
								}()
							}
							return btn.Layout(gtx)
						}),

						// Wyświetlenie odpowiedzi i tekstu z pola edytora
						layout.Rigid(func(gtx layout.Context) layout.Dimensions {
							return material.Body1(th, "Godziny: "+timeEditor.Text()).Layout(gtx)
						}),
						layout.Rigid(func(gtx layout.Context) layout.Dimensions {
							return material.Body1(th, "Odpowiedź ESP: "+response).Layout(gtx)
						}),

						layout.Rigid(func(gtx layout.Context) layout.Dimensions {
							btn1 := material.Button(th, &sendButton1, "Zaktualizuj czas")
							if sendButton1.Clicked(gtx) {
								go func() {
									var cmd string
									currentTime := time.Now()
									day := currentTime.Weekday().String()
									hour := currentTime.Hour()
									//TODO: hour must be 2 digits
									minute := currentTime.Minute()
									cmd = fmt.Sprintf("%s %s %d%d", "Date", day, hour, minute)
									resp, err := sendRawCommand(cmd)
									if err != nil {
										response = "Błąd: " + err.Error()
									} else {
										response = resp
									}
								}()
							}
							return btn1.Layout(gtx)
						}),
					)
				})

				evt.Frame(gtx.Ops)

			case app.DestroyEvent:
				os.Exit(0)
			}
		}
	}()
	app.Main()
}

const deviceIP = "192.168.1.65"

func sendRawCommand(cmd string) (string, error) {
	address := net.JoinHostPort(deviceIP, "80")
	conn, err := net.DialTimeout("tcp", address, 5*time.Second)
	if err != nil {
		return "", fmt.Errorf("connection failed: %w", err)
	}
	defer conn.Close()

	request := fmt.Sprintf("GET /%s", cmd)
	_, err = conn.Write([]byte(request))
	if err != nil {
		return "", fmt.Errorf("send failed: %w", err)
	}

	resp, err := io.ReadAll(conn)
	if err != nil {
		return "", fmt.Errorf("read failed: %w", err)
	}

	return string(resp), nil
}

func renderSlider(gtx layout.Context, th *material.Theme, slider *widget.Float, label string) layout.Dimensions {
	value := int(slider.Value * 100)

	return layout.Flex{
		Axis: layout.Vertical,
	}.Layout(gtx,
		layout.Rigid(func(gtx layout.Context) layout.Dimensions {
			return material.Body1(th, fmt.Sprintf("%s: %d", label, value)).Layout(gtx)
		}),
		layout.Rigid(func(gtx layout.Context) layout.Dimensions {
			return layout.Inset{Left: 16, Right: 16}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
				// maksymalna szerokość
				gtx.Constraints.Min.X = gtx.Constraints.Max.X
				return material.Slider(th, slider).Layout(gtx)
			})
		}),
	)
}
