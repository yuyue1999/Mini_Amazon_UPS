import smtplib
import socket
import selectors
import re
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
from email.mime.application import MIMEApplication

def send(email,username,packageid):
    # Sender and recipient email addresses
    sender_email = 'rideservices@outlook.com'
    recipient_email = email

# Create message container
    msg = MIMEMultipart()

# Add email content
    msg['From'] = sender_email
    msg['To'] = recipient_email
    msg['Subject'] = 'Your package has been delivered'

# Attach text content to email
    text = 'Dear '+username+'\n  Your package (package_id:'+packageid+') has been delivered. You can give your feedback through the following link:\n http://vcm-30639.vm.duke.edu:8000/feedback/'+packageid+'/'
    body = MIMEText(text)
    msg.attach(body)
    smtp_server = 'smtp.office365.com'
    smtp_port = 587
    smtp_username = 'rideservices@outlook.com'
    smtp_password = 'Yu19991011'
    with smtplib.SMTP(smtp_server, smtp_port) as server:
        server.starttls()
        server.login(smtp_username, smtp_password)
        server.sendmail(sender_email, recipient_email, msg.as_string())

if __name__ == "__main__":
    host = 'vcm-27827.vm.duke.edu'
    port = 8866
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    sel = selectors.DefaultSelector()
    sel.register(s, selectors.EVENT_READ, data=None)
    while True:
    # wait for events
        events = sel.select()

    # loop through the events
        for key, mask in events:
        # read the incoming data from the socket
            data = key.fileobj.recv(1024)
            if data:
            # print the incoming data
                message=data.decode('utf-8')
                email = re.search(r'email:(.*?)\$', message).group(1)
                packageid = re.search(r'packageid:(.*?)\$', message).group(1)
                username = re.search(r'username:(.*?)$', message).group(1)
                print("Email:", email)
                print("Package ID:", packageid)
                print("Username:", username)
                send(email,username,packageid)
                print('Received', data.decode('utf-8'))
            else:
            # remove the socket from the selector and close the connection
                sel.unregister(key.fileobj)
                key.fileobj.close()
                print('Connection closed')
                break


    
    