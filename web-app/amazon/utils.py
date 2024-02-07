import socket


def sendOrder(getAddrX, getAddrY, getUpsID, orderLists):
    # For test print
    # get the orderLists sizes
    for order in orderLists.all():
        itemId = order.itemID
        itemPrice = order.item_price
        itemDescription = order.name
        amount = order.amount
        customerName = order.userName
        orderInfo = getAddrX + ':' + getAddrY + ':' + str(amount) + ':' + str(itemId) + ':' + str(
            itemPrice) + ':' + itemDescription + ':' + customerName + ':' + getUpsID
        print('orderInfo is: ' + orderInfo)
        print(orderInfo.encode())
        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # connect to your server container
        # client.connect(('vcm-24822.vm.duke.edu', 9999))  # for changhao testing
        client.connect(('vcm-25941.vm.duke.edu', 9999)) #for junfeng testing
        print('connect to server success!')
        try:
            client.send(orderInfo.encode('utf-8'))
        except socket.error as e:
            print('Error sending data: %s' % e)

        print('after send')
        ACK_str = str(client.recv(1024))
