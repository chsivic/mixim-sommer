learning_received_relay = importdata('/home/chensi/omnetpp-4.2.1/samples/veins-2.0-rc1/examples/veins_tvws/learning_received_relay-1.csv');
random_received_relay = importdata('/home/chensi/omnetpp-4.2.1/samples/veins-2.0-rc1/examples/veins_tvws/random_received_relay-1.csv');


%%
ymatrix1=[learning_received_relay.data',random_received_relay.data']/2800*100;

% Create figure
figure1 = figure;

% Create axes
axes1 = axes('Parent',figure1,...
    'XTickLabel',{'1st Car','2nd Car','3rd Car','4th Car'},...
    'XTick',[1 2 3 4]);
% Uncomment the following line to preserve the X-limits of the axes
xlim(axes1,[1.5 4.5]);
grid(axes1,'on');
hold(axes1,'all');

% Create multiple lines using matrix input to bar
bar1 = bar(ymatrix1,'Parent',axes1);
set(bar1(1),'DisplayName','Learning-based');
set(bar1(2),'DisplayName','Random access');

% Create ylabel
ylabel('Correct reception of all packets (%)');

% Create legend
legend1 = legend(axes1,'show');

%%
learning_pkt_drop = importdata('/home/chensi/omnetpp-4.2.1/samples/veins-2.0-rc1/examples/veins_tvws/learning_pkt_drop-1.csv');
random_pkt_drop = importdata('/home/chensi/omnetpp-4.2.1/samples/veins-2.0-rc1/examples/veins_tvws/random_pkt_drop-1.csv');


%%
ymatrix1=[learning_pkt_drop.data',random_pkt_drop.data']/2800*100;

% Create figure
figure1 = figure;

% Create axes
axes1 = axes('Parent',figure1,...
    'XTickLabel',{'1st Car','2nd Car','3rd Car','4th Car'},...
    'XTick',[1 2 3 4]);
% Uncomment the following line to preserve the X-limits of the axes
xlim([0.5, 4.5]);
ylim([0,10]);
grid(axes1,'on');
hold(axes1,'all');

% Create multiple lines using matrix input to bar
bar1 = bar(ymatrix1,'Parent',axes1);
set(bar1(1),'DisplayName','Learning-based');
set(bar1(2),'DisplayName','Random access');

% Create ylabel
ylabel('Dropped packets due to congestion (%)');

% Create legend
legend1 = legend(axes1,'show');

