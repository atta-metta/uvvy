/* Main Window */
import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Window 2.0
import QtQuick.Controls 1.0

Window
{
    title: "Login"
    width: 300
    height: 300

    Column {
        anchors.centerIn: parent
        Image {
            id: metta
            source: "https://camo.githubusercontent.com/3793f6770550a407f10bfa3480d62fe720cb9d7f/68747470733a2f2f7261772e6769746875622e636f6d2f6265726b75732f6d657474612f6d61737465722f646f63732f6d657474612e706e67"
            sourceSize.width: 100
            asynchronous: true
            smooth: true
        }
        Label {
            text: "Username"
        }
        TextField {
            id: username
            anchors.margins: 2

            maximumLength: 50
            focus: true
            horizontalAlignment: TextInput.AlignHCenter
            onAccepted: { password.focus = true }
        }

        Rectangle {
            width : 50
            height : 10
            id: rect
            border.width: 0
            radius: 3
            color: "transparent"
        }

        Label {
            text: "Password"
        }

        TextField {
            id: password
            anchors.margins: 2
            focus: true
            echoMode : TextInput.Password
            horizontalAlignment: TextInput.AlignHCenter

            onAccepted: { login_button.clicked() }
        }

        Button {
            id: login_button

            text: "Login"
            enabled: { username.text.length > 0 && password.text.length > 0 }
            onClicked: { root.login(username.text, password.text); username.text = ""; password.text = "" }
        }

        Button {
            id: registration_button

            text: "Register"
            onClicked: { windowManager.openRegistrationWindow();}
        }

        Button {
            id: cancel

            text: "Cancel"
            onClicked: { root.exit(); }
        }
    }
}
